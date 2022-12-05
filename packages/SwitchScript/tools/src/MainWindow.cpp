#include "MainWindow.hpp"
#include "CVSignalWidget.hpp"
#include <qmediadevices.h>
#include <qtimer.h>
#include <qcamera.h>
#include <qmediacapturesession.h>
#include <qfiledialog.h>
#include <qimagecapture.h>
//#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) :
	QMainWindow(parent, flags), m_CurrentFile(std::nullopt), m_CameraIndex(std::nullopt)
{
	m_UI.setupUi(this);

	m_UI.spinSampleRate->setValue(30);

	m_Timer = new QTimer(this);
	m_Timer->setInterval(1000 / m_UI.spinSampleRate->value());

	auto videoInputs = QMediaDevices::videoInputs();
	m_Cameras.reserve(videoInputs.size());
	for (auto& device : videoInputs)
	{
		m_UI.comboVideoInputDevice->addItem(device.description());
		m_Cameras.push_back(new QCamera(device, this));
	}

	m_CameraCapture = new QMediaCaptureSession(this);
	m_CameraCapture->setVideoOutput(m_UI.videoPlayer);

	QImageCapture* imageCapture = new QImageCapture(m_CameraCapture);
	m_CameraCapture->setImageCapture(imageCapture);

	connect(m_UI.menuActionNew, &QAction::triggered, this, &MainWindow::OnNewFile);
	connect(m_UI.menuActionOpen, &QAction::triggered, this, &MainWindow::OnOpenFile);
	connect(m_UI.menuActionSave, &QAction::triggered, this, &MainWindow::OnSaveFile);
	connect(m_UI.menuActionSaveAs, &QAction::triggered, this, &MainWindow::OnSaveAsFile);

	connect(m_Timer, &QTimer::timeout, imageCapture, &QImageCapture::capture);
	connect(imageCapture, &QImageCapture::imageCaptured, this, &MainWindow::OnProcessFrame);

	connect(m_UI.toolBarActionRun, &QAction::triggered, this, &MainWindow::OnStartCapture);
	connect(m_UI.toolbarActionStop, &QAction::triggered, this, &MainWindow::OnStopCapture);

	connect(m_UI.buttonAddSignal, &QPushButton::clicked, this, &MainWindow::OnAddSignal);
	connect(m_UI.buttonRemoveSignal, &QPushButton::clicked, this, &MainWindow::OnRemoveSignal);

	connect(m_UI.comboVideoInputDevice, &QComboBox::currentIndexChanged, this, &MainWindow::OnChangeCamera);

	if (videoInputs.size() > 0)
		OnChangeCamera(m_UI.comboVideoInputDevice->currentIndex());
}

void MainWindow::OnNewFile()
{
	
}

void MainWindow::OnOpenFile()
{

}

void MainWindow::OnSaveFile()
{

}

void MainWindow::OnSaveAsFile()
{

}

void MainWindow::OnProcessFrame(int id, const QImage& preview)
{
	/*
	cv::Mat previewMat;
	if (preview.format() == QImage::Format::Format_Grayscale8)
	{
		previewMat = cv::Mat(preview.height(), preview.width(), CV_8UC1, (void*)preview.constBits(), preview.bytesPerLine());
	}
	else
	{
		QImage temp = preview.convertToFormat(QImage::Format::Format_Grayscale8);
		previewMat = cv::Mat(temp.height(), temp.width(), CV_8UC1, (void*)temp.constBits(), temp.bytesPerLine());
	}

	*/
}

void MainWindow::OnAddSignal()
{
	QListWidgetItem* item = new QListWidgetItem(m_UI.signalList);
	CVSignalWidget* widget = new CVSignalWidget(m_UI.signalList);
	m_UI.signalList->addItem(item);
	item->setSizeHint(widget->minimumSizeHint());
	m_UI.signalList->setItemWidget(item, widget);
}

void MainWindow::OnRemoveSignal()
{
	qDeleteAll(m_UI.signalList->selectedItems());
}

void MainWindow::OnStartCapture()
{
	m_UI.tabScript->setDisabled(true);
	m_UI.groupSignalSetting->setDisabled(true);
	m_Timer->start();
}

void MainWindow::OnStopCapture()
{
	m_Timer->stop();
	m_UI.groupSignalSetting->setDisabled(false);
	m_UI.tabScript->setDisabled(false);
}

void MainWindow::OnChangeCamera(int newIndex)
{
	if (m_CameraIndex.has_value())
		m_Cameras[m_CameraIndex.value()]->stop();

	m_CameraIndex = newIndex;
	m_CameraCapture->setCamera(m_Cameras[newIndex]);
	m_Cameras[newIndex]->start();
}