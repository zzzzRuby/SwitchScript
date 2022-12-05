#pragma once

#include <ui_CVSignalWidget.h>
#include <qwidget.h>

class CVSignalWidget : public QWidget
{
    Q_OBJECT
private:
	Ui::CVSignalWidget m_UI;
	QRect m_Area;
	uint16_t m_Signal;
	QByteArray m_ImageBuffer;
public:
	CVSignalWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
signals:
	bool SelectArea(QRect& rect);
	void AreaChanged(const QRect& rect);
	void SignalChanged(uint16_t signal);
private slots:
	void OnSelectImage();
	void OnSelectArea();

	void OnXChanged(int value);
	void OnYChanged(int value);
	void OnWidthChanged(int value);
	void OnHeightChanged(int value);
	void OnSignalChanged(int value);
};