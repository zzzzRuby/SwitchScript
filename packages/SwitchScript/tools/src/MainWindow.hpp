#pragma once

#include <optional>
#include <qmainwindow.h>
#include <ui_MainWindow.h>
#include "SSML.hpp"

class QTimer;
class QCamera;
class QMediaCaptureSession;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
	Ui::MainWindow m_UI;
	QTimer* m_Timer;
	std::vector<QCamera*> m_Cameras;
	QMediaCaptureSession* m_CameraCapture;
	std::optional<int> m_CameraIndex;
	std::optional<QString> m_CurrentFile;
public:
	MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
private slots:
	void OnNewFile();
	void OnOpenFile();
	void OnSaveFile();
	void OnSaveAsFile();

	void OnAddSignal();
	void OnRemoveSignal();

	void OnProcessFrame(int id, const QImage& preview);

	void OnStartCapture();
	void OnStopCapture();

	void OnChangeCamera(int newIndex);
};