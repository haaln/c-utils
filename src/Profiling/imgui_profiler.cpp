#include "imgui.h"

void profiler_plot(const char *mainPlotName, float scale) {
  if (history.empty()) {
    return;
  }

  ImGui::PushID(mainPlotName);

  ImGui::Checkbox("Pause", &pause);
  ImGui::SameLine();

  const char *names[16] = {mainPlotName}; // todo

  int counter = 0;
  for (auto &p : subProfiles) {
    names[subProfiles.size() - (counter++)] = p.first.c_str();
  }

  const ImColor colors[16] = {
      {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.5f, 1.0f},
      {0.1f, 0.3f, 0.9f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f},
      {0.7f, 0.6f, 1.0f, 1.0f}, {0.6f, 0.0f, 0.3f, 1.0f},
      {0.9f, 0.1f, 0.0f, 1.0f}, {0.0f, 0.5f, 0.5f, 1.0f},
      {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f},
      {0.3f, 1.0f, 0.3f, 1.0f}, {0.3f, 0.3f, 1.0f, 1.0f},
      {1.0f, 0.5f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.0f, 1.0f},
      {0.3f, 0.9f, 0.5f, 1.0f}, {0.5f, 0.5f, 1.0f, 1.0f},
  }; // todo

  ImGui::Text(mainPlotName);

  ImGui::PlotMultiHistograms(mainPlotName,           // label
                             subProfiles.size() + 1, // num_hists,
                             names,                  // names,
                             colors,                 // colors,
                             plotGetter,             // getter
                             plotGetterReal,         // getter
                             &history,               // datas,
                             history.size(),         // values_count,
                             0.f,                    // scale_min,
                             scale,                  // scale_max,
                             ImVec2(512.0, 117.0f)   // graph_size
  );

  ImGui::Text("Average:");
  for (int index = 0; index < subProfiles.size() + 1; index++) {
    float average = 0;
    for (int i = 0; i < history.size(); i++) {
      average += history[i].dataMsReal[index];
    }
    average /= history.size();

    ImGui::ColorButton("X", colors[index], ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    ImGui::Text("%s: %f ms", names[index], average);
  }

  ImGui::PopID();
}
