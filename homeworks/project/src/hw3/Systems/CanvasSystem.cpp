#include "CanvasSystem.h"

#include "../Components/CanvasData.h"

#include <_deps/imgui/imgui.h>
#include<iostream>
#include<string>
#include<map>
#include"mylib.h"
using namespace Ubpa;

void CanvasSystem::OnUpdate(Ubpa::UECS::Schedule& schedule) {

	schedule.RegisterCommand([](Ubpa::UECS::World* w) {
		auto data = w->entityMngr.GetSingleton<CanvasData>();
		if (!data)
			return;
		if (ImGui::Begin("Canvas")) {
			ImGui::Checkbox("Enable grid", &data->opt_enable_grid);
			ImGui::SameLine(250.f,-1.f);
			ImGui::Checkbox("Enable context menu", &data->opt_enable_context_menu);
			std::map<std::string, bool>::iterator it,itEnd,it2,itEnd2;


			it = data->bases.begin();
			itEnd = data->bases.end();
			int count = 0;
			while (it != itEnd) {
				ImGui::Checkbox(it->first.c_str(), &it->second);
				if(count++!=data->bases.size()-1)
				ImGui::SameLine(0, 100.f);
				it++;
			}


			it = data->fit_ways.begin();
			itEnd = data->fit_ways.end();
			count = 0;
			while (it != itEnd) {
				ImGui::Checkbox(it->first.c_str(), &it->second);
				if(count++!=data->fit_ways.size()-1)
				ImGui::SameLine(0, 100.f);
				it++;
			}


			it = data->parameterization.begin();
			itEnd = data->parameterization.end();
			count = 0;
			while (it != itEnd) {
				ImGui::Checkbox(it->first.c_str(), &it->second);
				if (count++ != data->parameterization.size() - 1)
					ImGui::SameLine(0, 100.f);
				it++;
			}


			ImGui::DragInt("Ceta", &data->ceta, 1.f, 1, 200, "%d", 0);
			ImGui::InputFloat("Lambda", &data->lambda,0.001,0.01,3,0);
			ImGui::DragInt("max", &data->max, 1.f, 1, 200, "%d", 0);
			ImGui::Text("mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
			// Typically you would use a 
			Child()/EndChild() pair to benefit from a clipping region + own scrolling.
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
			if (!data->changing_point)
				data->index = getNearest1(data->points, mouse_pos_in_canvas, 20.f);
			if (is_hovered && !data->changing_point && ImGui::IsMouseClicked(ImGuiMouseButton_Left)&&data->index==-1 )
			{
				data->points.push_back(mouse_pos_in_canvas);
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				data->changing_point = true;
			if (data->changing_point&& data->index != -1) {
	
					data->points[data->index] = mouse_pos_in_canvas;
			}
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
				data->changing_point = false;
				

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
					data->points.resize(data->points.size() - 2);
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
			if (data->points.size() >= 2) {
				std::map<std::string, bool>::iterator it_base, itEnd_base, it_fit, itEnd_fit,it_param,itEnd_param;
				it_base = data->bases.begin();
				itEnd_base = data->bases.end();
				while (it_base != itEnd_base) {
					it_fit = data->fit_ways.begin();
					itEnd_fit = data->fit_ways.end();
					while (it_fit != itEnd_fit) {
						it_param = data->parameterization.begin();
						itEnd_param = data->parameterization.end();
						while (it_param != itEnd_param) {
							if (it_base->second && it_fit->second&&it_param->second) {
								adopt ex(*data, it_base->first, it_fit->first,it_param->first);
								for (float i = 0; i < 1.f; i += 0.01) {
									draw_list->AddLine(ImVec2(ex.getResult(i)[0] + origin.x, ex.getResult(i)[1] + origin.y), ImVec2(ex.getResult(i + 0.01)[0] + origin.x, ex.getResult(i + 0.01)[1] + origin.y), IM_COL32(ex.col.x, ex.col.y, ex.col.z, ex.col.w), 3.5f);
								}
							}
							it_param++;
						}
						it_fit++;
					}
					it_base++;
				}
			}
			for (int n = 0; n < data->points.size(); n += 1) {
				draw_list->AddCircle(ImVec2(origin.x + data->points[n][0], origin.y + data->points[n][1]), 3, IM_COL32(255, 255, 0, 255), 0,5.f);
			}

			draw_list->PopClipRect();
		}

		ImGui::End();
	});
}
