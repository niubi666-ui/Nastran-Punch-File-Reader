#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include "dtknastrangraphpchdatastore.h"
#include "dtknastrangraphpchparser.h"

// 简单的翻译函数，模拟 UI 层名称转换
std::string translateComp(Component comp) {
    switch (comp) {
    case Component::SX: return "SX (Normal Stress/Strain)";
    case Component::T1: return "T1 (Translation X)";
    case Component::STRAIN_ENERGY: return "Strain Energy";
    case Component::VON_MISES: return "Von Mises";
    default: return "Other Component";
    }
}

std::string translateLoc(LocationType loc) {
    switch (loc) {
    case LocationType::Z1: return "Z1 (Top)";
    case LocationType::Z2: return "Z2 (Bottom)";
    case LocationType::CENTER: return "Center";
    default: return "Single";
    }
}

void plot(std::vector<double> xCoords, std::vector<double> yCoords)
{
    // Save an interactive HTML plot (Chart.js) so the curve can be viewed in a browser
    auto savePlotHtml = [](const std::string& filename,
        const std::vector<double>& x,
        const std::vector<double>& y,
        const std::string& xLabel = "X",
        const std::string& yLabel = "Y") {
            std::ofstream ofs(filename);
            if (!ofs.is_open()) return false;

            std::ostringstream xs;
            std::ostringstream ys;
            xs << std::fixed << std::setprecision(6);
            ys << std::fixed << std::setprecision(6);

            xs << "[";
            ys << "[";
            for (size_t i = 0; i < x.size(); ++i) {
                if (i) { xs << ", "; ys << ", "; }
                xs << x[i];
                ys << y[i];
            }
            xs << "]";
            ys << "]";

            std::string html =
                "<!doctype html>\n"
                "<html>\n"
                "<head>\n"
                "  <meta charset=\"utf-8\">\n"
                "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                "  <title>Curve Plot</title>\n"
                "  <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n"
                "</head>\n"
                "<body>\n"
                "  <h3>Curve Plot</h3>\n"
                "  <canvas id=\"chart\" width=\"800\" height=\"400\"></canvas>\n"
                "  <script>\n"
                "    const ctx = document.getElementById('chart').getContext('2d');\n"
                "    const dataX = " + xs.str() + ";\n"
                "    const dataY = " + ys.str() + ";\n"
                "    const labels = dataX.map(v => v.toString());\n"
                "    const chart = new Chart(ctx, {\n"
                "      type: 'line',\n"
                "      data: {\n"
                "        labels: labels,\n"
                "        datasets: [{\n"
                "          label: '" + yLabel + " vs " + xLabel + "',\n"
                "          data: dataY,\n"
                "          fill: false,\n"
                "          borderColor: 'rgba(75, 192, 192, 1)',\n"
                "          tension: 0.1\n"
                "        }]\n"
                "      },\n"
                "      options: {\n"
                "        responsive: true,\n"
                "        scales: {\n"
                "          x: {\n"
                "            display: true,\n"
                "            title: { display: true, text: '" + xLabel + "' }\n"
                "          },\n"
                "          y: {\n"
                "            display: true,\n"
                "            title: { display: true, text: '" + yLabel + "' }\n"
                "          }\n"
                "        }\n"
                "      }\n"
                "    });\n"
                "  </script>\n"
                "</body>\n"
                "</html>\n";

            ofs << html;
            ofs.close();
            return true;
        };

    const std::string outFile = "curve_plot.html";
    if (savePlotHtml(outFile, xCoords, yCoords, "X", "Y")) {
        std::cout << "Saved interactive plot to '" << outFile << "'. Open it in a browser to view the curve." << std::endl;
        // Try to open the HTML file with the default system application (browser)
        auto openWithDefaultApp = [](const std::string& filePath) {
            // First attempt: ShellExecute (Windows API)
            HINSTANCE res = ShellExecuteA(NULL, "open", filePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
            if ((intptr_t)res <= 32) {
                // Fallback: use system start command
                std::string cmd = "start \"\" \"" + filePath + "\"";
                std::system(cmd.c_str());
            }
            };

        openWithDefaultApp(outFile);
    }
    else {
        std::cout << "Failed to save HTML plot file." << std::endl;
    }
}

std::string openPchFileDialog()
{
    char fileName[MAX_PATH] = { 0 };
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "pch Files\0*.pch\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Select pch file";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(fileName);
    }
    return std::string();
}

int main()
{
    // 1. 初始化数据池和解析器
    PchDataStore store;
    PchParser parser(store);

    // 2. 指定 PCH 文件路径进行解析
    //std::string pchPath = "pch_test/K11_TB_IPI_20180122.pch";
    std::string pchPath = openPchFileDialog();
    if (pchPath.empty()) {
        std::cerr << "No file selected. Exiting." << std::endl;
        return -1;
    }

    if (!parser.parse(pchPath)) {
        std::cerr << "Failed to open or parse PCH file!" << std::endl;
        return -1;
    }

    std::vector<double> xCoords;
    std::vector<double> yCoords;

    // 参数：Subcase, ElementType, ParentID, GridID, Location, Component
	// 把你想要绘制曲线数据的维度信息填在这里，然后会绘制出曲线图。注意要和你解析的文件中的数据维度匹配，否则可能找不到数据。
    store.getCurveData(1, 0, 3261, 0, LocationType::CENTER, Component::T1, xCoords, yCoords);

    if (xCoords.empty())
    {
        std::cout << "No data found for the specified criteria." << std::endl;
    }
    else
    {
        for (size_t i = 0; i < xCoords.size(); ++i)
        {
            std::cout << "  X: " << xCoords[i] << " \t Y: " << yCoords[i] << std::endl;
        }

        plot(xCoords, yCoords);
    }

    std::cout << "\n--- Test Completed ---" << std::endl;
    return 0;
}