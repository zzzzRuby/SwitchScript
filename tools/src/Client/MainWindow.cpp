#include "MainWindow.hpp"

MainWindow::MainWindow() :
	m_UI { }, m_VideoCapture(0)
{
	m_UI.setupUi(this);
}