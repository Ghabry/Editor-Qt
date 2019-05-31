#include "stat_curve_item.h"
#include "QGraphicsScene"

CurveItem::CurveItem(QColor color, std::vector<int16_t> &data, QGraphicsItem *parent) :
	QGraphicsItem(parent),
	m_data(data)
{
	m_color = color;
	m_maxValue = 999.0;
}

QRectF CurveItem::boundingRect() const
{
	if (!scene())
		return QRectF();

	return scene()->sceneRect();
}

void CurveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (m_data.empty() || !scene())
		return;

	QRectF r = scene()->sceneRect();
	QVector<QPointF> p;
	p.append(r.bottomLeft());
	for (size_t i = 0; i < m_data.size(); i++)
		p.append(QPointF((static_cast<qreal>(i)*r.width())/99.0,
						 r.height()-r.height()*static_cast<qreal>(m_data[i])/m_maxValue));
	p.append(r.bottomRight());

	painter->fillRect(r, Qt::black);
	painter->setBrush(QBrush(m_color));
	painter->drawPolygon(QPolygonF(p));
	painter->setPen(QPen(m_color.light()));
	painter->drawPoints(p);
}

void CurveItem::setData(std::vector<int16_t> &data)
{
	m_data = data;
	update();
}
qreal CurveItem::maxValue() const
{
	return m_maxValue;
}

void CurveItem::setMaxValue(const qreal &maxValue)
{
	m_maxValue = maxValue;
}



