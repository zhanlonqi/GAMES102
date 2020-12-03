#include "CanvasSystem.h"

#include "../Components/CanvasData.h"

#include <_deps/imgui/imgui.h>

using namespace Ubpa;

std::vector<pointf2> chaiukin2(std::vector<pointf2> points, int num);
std::vector<pointf2> chaiukin3(std::vector<pointf2> points, int num);
std::vector<pointf2> interpolation(std::vector<pointf2> points, int num);
bool Chaiukin2 = false;
bool Chaiukin3 = false;
bool Interpolation = false;
int num_iterate;
void CanvasSystem::OnUpdate(Ubpa::UECS::Schedule& schedule) {
	schedule.RegisterCommand([](Ubpa::UECS::World* w) {
		auto data = w->entityMngr.GetSingleton<CanvasData>();
		if (!data)
			return;

		if (ImGui::Begin("Canvas")) {
			ImGui::Checkbox("Enable grid", &data->opt_enable_grid);
			ImGui::Checkbox("Enable context menu", &data->opt_enable_context_menu);
			ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
			ImGui::DragInt("Num_iterate", &num_iterate, 1, 0, 10, "%d", 0);
			ImGui::Checkbox("Chaiukin2 ", &Chaiukin2);
			ImGui::SameLine(0, 0);
			ImGui::Checkbox("Chaiukin3 ", &Chaiukin3);
			ImGui::SameLine(0, 0);
			ImGui::Checkbox("Interpolation ", &Interpolation);
			// Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
			// Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
			// To use a child window instead we could use, e.g:
			//      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
			//      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
			//      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
			//      ImGui::PopStyleColor();
			//      ImGui::PopStyleVar();
			//      [...]
			//      ImGui::EndChild();

			// Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
			if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
			if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
			ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

			// Draw border and background color
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
			draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

			// This will catch our interactions
			ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
			const bool is_hovered = ImGui::IsItemHovered(); // Hovered
			const bool is_active = ImGui::IsItemActive();   // Held
			const ImVec2 origin(canvas_p0.x + data->scrolling[0], canvas_p0.y + data->scrolling[1]); // Lock scrolled origin
			const pointf2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

			// Add first and second point
			if (is_hovered  && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				data->points.push_back(mouse_pos_in_canvas);
			}

			// Pan (we use a zero mouse threshold when there's no context menu)
			// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
			const float mouse_threshold_for_pan = data->opt_enable_context_menu ? -1.0f : 0.0f;
			if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
			{
				data->scrolling[0] += io.MouseDelta.x;
				data->scrolling[1] += io.MouseDelta.y;
			}

			// Context menu (under default mouse threshold)
			ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (data->opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
				ImGui::OpenPopupContextItem("context");
			if (ImGui::BeginPopup("context"))
			{
				if (data->adding_line)
					data->points.resize(data->points.size() - 1);
				data->adding_line = false;
				if (ImGui::MenuItem("Remove one", NULL, false, data->points.size() > 0)) { data->points.resize(data->points.size() - 1); }
				if (ImGui::MenuItem("Remove all", NULL, false, data->points.size() > 0)) { data->points.clear(); }
				ImGui::EndPopup();
			}

			// Draw grid + all lines in the canvas
			draw_list->PushClipRect(canvas_p0, canvas_p1, true);
			if (data->opt_enable_grid)
			{
				const float GRID_STEP = 64.0f;
				for (float x = fmodf(data->scrolling[0], GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
				for (float y = fmodf(data->scrolling[1], GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
			}
			std::vector<pointf2> new_points;
			std::vector<pointf2> new_points2;
			std::vector<pointf2> new_points3;
			if (data->points.size() >= 2) {
				if(Chaiukin2)
				new_points = chaiukin2(data->points, num_iterate);
				if(Chaiukin3)
				new_points2 = chaiukin3(data->points, num_iterate);
				if (Interpolation)
					new_points3 = interpolation(data->points, num_iterate);
			}
			for (int n = 0; n < data->points.size()&&data->points.size()>=1 ; n += 1) {
				draw_list->AddCircle(ImVec2(origin.x + data->points[n][0], origin.y + data->points[n][1]), 3, IM_COL32(255, 25, 0, 255), 0, 2);
			}


			for (int n = 0; n < new_points.size(); n++) {
				draw_list->AddCircle(ImVec2(origin.x + new_points[n][0], origin.y + new_points[n][1]), 3, IM_COL32(255, 255, 0, 255), 0, 5);
			}
			for (int n = 0; n < new_points2.size(); n++) {
				draw_list->AddCircle(ImVec2(origin.x + new_points2[n][0], origin.y + new_points2[n][1]), 3, IM_COL32(255, 255, 0, 255), 0, 5);
			}
			for (int n = 0; n < new_points3.size(); n++) {
				draw_list->AddCircle(ImVec2(origin.x + new_points3[n][0], origin.y + new_points3[n][1]), 3, IM_COL32(255, 255, 0, 255), 0, 5);
			}


			for (int n = 0; n < new_points.size() - 1 && data->points.size() >= 2&&Chaiukin2; n++) {
				draw_list->AddLine(ImVec2(origin.x +new_points[n][0], origin.y + new_points[n][1]), ImVec2(origin.x + new_points[n + 1][0], origin.y + new_points[n + 1][1]), IM_COL32(255, 0, 255, 255), 2);
			}
			for (int n = 0; n < new_points2.size() - 1 && data->points.size() >= 2&&Chaiukin3; n++) {
				draw_list->AddLine(ImVec2(origin.x + new_points2[n][0], origin.y + new_points2[n][1]), ImVec2(origin.x + new_points2[n + 1][0], origin.y + new_points2[n + 1][1]), IM_COL32(255, 50, 25, 255), 2);
			}
			for (int n = 0; n < new_points3.size() - 1 && data->points.size() >= 4 && Interpolation; n++) {
				draw_list->AddLine(ImVec2(origin.x + new_points3[n][0], origin.y + new_points3[n][1]), ImVec2(origin.x + new_points3[n + 1][0], origin.y + new_points3[n + 1][1]), IM_COL32(155, 1050, 25, 255), 2);
			}
			
			draw_list->PopClipRect();
		}

		ImGui::End();
	});
}

std::vector<pointf2> chaiukin2(std::vector<pointf2> points, int num) {
	if (num == 0) {
		return points;
	}
	std::vector<pointf2> new_points;
	for (int i = 0; i < points.size()-1; i++) {
		new_points.push_back(pointf2(points[i][0] * 0.75 + points[i + 1][0] * 0.25, points[i][1] * 0.75 + points[i + 1][1] * 0.25));
		new_points.push_back(pointf2(points[i+1][0] * 0.75 + points[i ][0] * 0.25, points[i+1][1] * 0.75 + points[i ][1] * 0.25));
	}
	return chaiukin2(new_points, num - 1);
}

std::vector<pointf2> chaiukin3(std::vector<pointf2> points, int num) {
	if (num == 0) {
		return points;
	}
	std::vector<pointf2>  new_points;
	new_points.push_back(points[0]);
	for (int i = 1; i < points.size()-1; i++)
	{
		new_points.push_back(pointf2(0.5 * points[i - 1][0] + 0.5 * points[i][0], 0.5 * points[i-1][1] + 0.5 * points[i][1]));
		new_points.push_back(pointf2(0.125 * points[i - 1][0] + 0.75 * points[i][0] + 0.125 * points[i+1][0], 0.125 * points[i - 1][1] + 0.75 * points[i][1] + 0.125 * points[i+1][1]));
	}
	new_points.push_back(points[points.size() - 1]);
	return chaiukin3(new_points, num - 1);
}

std::vector<pointf2> interpolation(std::vector<pointf2> points, int num) {
	if (num == 0||points.size()<4) {
		return points;
	}
	std::vector<pointf2> new_points;
		new_points.push_back(points[0]);
	for (int i = 1; i < points.size() - 2;i+=1) {
		new_points.push_back(points[i]);
		pointf2 mid = pointf2(points[i ][0] * 0.5 + points[i + 1][0] * 0.5, points[i ][1] * 0.5 + points[i + 1][1] * 0.5);
		pointf2 mid2 = pointf2(points[i - 1][0] * 0.5 + points[i + 2][0] * 0.5, points[i - 1][1] * 0.5 + points[i + 2][1] * 0.5);
		pointf2 dir = pointf2((mid[0] - mid2[0])*0.1, (mid[1] - mid2[1])*0.1);
		pointf2 point = pointf2(mid[0] + dir[0], mid[1] + dir[1]);
		new_points.push_back(point);
	}
	new_points.push_back(points[points.size() - 2]);
	new_points.push_back(points[points.size() - 1]);
	return interpolation(new_points, num - 1);
}
