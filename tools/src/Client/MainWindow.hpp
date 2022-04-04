#pragma once

#include <qmainwindow.h>
#include <Client/ui_MainWindow.h>
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
private:
	Ui::MainWindow m_UI;
	cv::VideoCapture m_VideoCapture;
public:
	MainWindow();
};