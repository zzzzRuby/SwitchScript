#include "CVSignalWidget.hpp"
#include <qfiledialog.h>

CVSignalWidget::CVSignalWidget(QWidget* parent, Qt::WindowFlags f) :
	QWidget(parent, f), m_Signal(0)
{
	m_UI.setupUi(this);

	connect(m_UI.buttonImage, &QPushButton::clicked, this, &CVSignalWidget::OnSelectImage);
	connect(m_UI.buttonSelect, &QPushButton::clicked, this, &CVSignalWidget::OnSelectArea);
	connect(m_UI.spinX, &QSpinBox::valueChanged, this, &CVSignalWidget::OnXChanged);
	connect(m_UI.spinY, &QSpinBox::valueChanged, this, &CVSignalWidget::OnYChanged);
	connect(m_UI.spinWidth, &QSpinBox::valueChanged, this, &CVSignalWidget::OnWidthChanged);
	connect(m_UI.spinHeight, &QSpinBox::valueChanged, this, &CVSignalWidget::OnHeightChanged);

	connect(m_UI.spinSignal, &QSpinBox::valueChanged, this, &CVSignalWidget::OnSignalChanged);
}

void CVSignalWidget::OnSelectImage()
{
	auto imageFile = QFileDialog::getOpenFileName(this, tr("Select Image"), QString(), 
		tr("Image Files(*.jpg *.png *.bmp)"));
	if (!imageFile.isEmpty() && !imageFile.isNull())
	{
		QFile f(imageFile);
		f.open(QIODeviceBase::OpenModeFlag::ReadOnly);
		m_ImageBuffer = f.readAll();

		QPixmap pixmap;
		pixmap.loadFromData(m_ImageBuffer);
		QIcon buttonIcon(pixmap);
		m_UI.buttonImage->setText(QStringLiteral(""));
		m_UI.buttonImage->setIcon(buttonIcon);
	}
}

void CVSignalWidget::OnSelectArea()
{
	bool result = emit SelectArea(m_Area);
	if (!result)
		return;

	disconnect(m_UI.spinX, &QSpinBox::valueChanged, this, &CVSignalWidget::OnXChanged);
	disconnect(m_UI.spinY, &QSpinBox::valueChanged, this, &CVSignalWidget::OnYChanged);
	disconnect(m_UI.spinWidth, &QSpinBox::valueChanged, this, &CVSignalWidget::OnWidthChanged);
	disconnect(m_UI.spinHeight, &QSpinBox::valueChanged, this, &CVSignalWidget::OnHeightChanged);
	m_UI.spinX->setValue(m_Area.x());
	m_UI.spinY->setValue(m_Area.y());
	m_UI.spinWidth->setValue(m_Area.width());
	m_UI.spinHeight->setValue(m_Area.height());
	connect(m_UI.spinX, &QSpinBox::valueChanged, this, &CVSignalWidget::OnXChanged);
	connect(m_UI.spinY, &QSpinBox::valueChanged, this, &CVSignalWidget::OnYChanged);
	connect(m_UI.spinWidth, &QSpinBox::valueChanged, this, &CVSignalWidget::OnWidthChanged);
	connect(m_UI.spinHeight, &QSpinBox::valueChanged, this, &CVSignalWidget::OnHeightChanged);

	emit AreaChanged(m_Area);
}

void CVSignalWidget::OnXChanged(int value)
{
	m_Area.setX(value);
	emit AreaChanged(m_Area);
}

void CVSignalWidget::OnYChanged(int value)
{
	m_Area.setY(value);
	emit AreaChanged(m_Area);
}

void CVSignalWidget::OnWidthChanged(int value)
{
	m_Area.setWidth(value);
	emit AreaChanged(m_Area);
}

void CVSignalWidget::OnHeightChanged(int value)
{
	m_Area.setHeight(value);
	emit AreaChanged(m_Area);
}

void CVSignalWidget::OnSignalChanged(int value)
{
	m_Signal = (uint16_t)value;
	emit SignalChanged(m_Signal);
}