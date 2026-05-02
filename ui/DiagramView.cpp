#include "DiagramView.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainterPath>
#include <QPainter>
#include <QWheelEvent>
#include <QApplication>
#include <QScrollBar>
#include <cmath>
#include <map>

static constexpr double NODE_R        = 36.0;   
static constexpr double LAYOUT_RADIUS = 200.0;  
static constexpr double ARROW_HEAD    = 10.0;   
static constexpr double CTRL_OFFSET   = 80.0;   

static QPointF pointOnCircle(QPointF center, double r, double angleDeg) {
    double rad = angleDeg * M_PI / 180.0;
    return center + QPointF(r * std::cos(rad), r * std::sin(rad));
}

static void drawArrowHead(QPainterPath& path, QPointF tip, QPointF from) {
    QPointF dir  = tip - from;
    double  len  = std::sqrt(dir.x()*dir.x() + dir.y()*dir.y());
    if (len < 1e-6) return;
    dir /= len;
    QPointF perp(-dir.y(), dir.x());
    QPointF base = tip - dir * ARROW_HEAD;
    path.moveTo(tip);
    path.lineTo(base + perp * (ARROW_HEAD * 0.4));
    path.lineTo(base - perp * (ARROW_HEAD * 0.4));
    path.closeSubpath();
}

class ZoomableView : public QGraphicsView {
public:
    explicit ZoomableView(QGraphicsScene* scene, QWidget* parent = nullptr)
        : QGraphicsView(scene, parent) {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

protected:
    void wheelEvent(QWheelEvent* event) override {
        const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
        scale(factor, factor);
        event->accept();
    }
};


DiagramScene::DiagramScene(QObject* parent) : QGraphicsScene(parent) {}

QPointF DiagramScene::circlePosition(int index, int total, double radius, QPointF center) {
    double angle = -90.0 + (360.0 / total) * index;
    return pointOnCircle(center, radius, angle);
}

void DiagramScene::setMachine(const std::vector<State>& states, const QString& /*title*/) {
    QGraphicsScene::clear();
    if (states.empty()) return;

    int n = (int)states.size();
    QPointF center(0, 0);

    
    double radius = LAYOUT_RADIUS;
    if (n > 6) radius = LAYOUT_RADIUS * (1.0 + 0.15 * (n - 6));

    std::map<char, QPointF> positions;
    std::map<char, QPointF> centers;

    for (int i = 0; i < n; i++) {
        QPointF pos = circlePosition(i, n, radius, center);
        positions[states[i].label] = pos;
        centers[states[i].label]   = pos + QPointF(NODE_R, NODE_R);
    }

    std::map<std::pair<char,char>, int> pairCount;

    for (auto& s : states) {
        for (int bit = 0; bit < 2; bit++) {
            char to = s.ns[bit];
            auto key = std::make_pair(std::min(s.label, to), std::max(s.label, to));
            pairCount[key]++;
        }
    }

    std::map<std::pair<char,char>, int> pairSeen;

    for (auto& s : states) {
        for (int bit = 0; bit < 2; bit++) {
            char from  = s.label;
            char to    = s.ns[bit];
            QString lbl = QString("%1/%2").arg(bit).arg(s.output);

            QPointF fc = centers[from];
            QPointF tc = centers[to];

            QPainterPath arrowPath;
            QPointF      labelPos;

            if (from == to) {
                
                double angle = -90.0 + (360.0 / n) *
                    std::distance(states.begin(),
                        std::find_if(states.begin(), states.end(),
                            [from](const State& st){ return st.label == from; }));
                QPointF loopAnchor = pointOnCircle(fc, NODE_R, angle);
                QPointF cp1 = loopAnchor + QPointF(-35, -50);
                QPointF cp2 = loopAnchor + QPointF( 35, -50);

                arrowPath.moveTo(loopAnchor);
                arrowPath.cubicTo(cp1, cp2, loopAnchor);

                QPointF tangent = loopAnchor - cp2;
                drawArrowHead(arrowPath, loopAnchor, loopAnchor - tangent * 0.01);

                labelPos = loopAnchor + QPointF(-10, -65);

            } else {
                // ── Straight offset lines ─────────────────────────────────
                auto key = std::make_pair(std::min(from, to), std::max(from, to));
                int total = pairCount[key];
                int seen  = pairSeen[key]++;

                // Direction vector from->to
                QPointF dir = tc - fc;
                double  len = std::sqrt(dir.x()*dir.x() + dir.y()*dir.y());
                QPointF norm = dir / len;
                QPointF perp(-norm.y(), norm.x());

                // Offset: if two arrows share the same pair, shift each to one side
                double offsetAmt = (total > 1) ? 5.0 : 0.0;
                double offsetDir = (seen == 0) ? 1.0 : -1.0;
                QPointF offset   = perp * offsetAmt * offsetDir;

                // Entry/exit points on node circumferences, shifted sideways
                QPointF exitPt  = fc + norm * NODE_R + offset;
                QPointF entryPt = tc - norm * NODE_R + offset;

                arrowPath.moveTo(exitPt);
                arrowPath.lineTo(entryPt);

                // Arrowhead
                drawArrowHead(arrowPath, entryPt, exitPt);

                // Label sits at midpoint, offset sideways a little
                QPointF mid = (exitPt + entryPt) * 0.5;
                labelPos = mid + perp * offsetDir * 12.0;
            }

            auto* pathItem = addPath(arrowPath);
            pathItem->setPen(Qt::NoPen);

            QPainterPath strokeOnly;
            strokeOnly.moveTo(arrowPath.elementAt(0));
            for (int e = 1; e < arrowPath.elementCount(); e++) {
                auto el = arrowPath.elementAt(e);
                if (el.type == QPainterPath::MoveToElement) break;
                if (el.type == QPainterPath::LineToElement)
                    strokeOnly.lineTo(el);
                else if (el.type == QPainterPath::CurveToElement) {
                    auto e1 = arrowPath.elementAt(e);
                    auto e2 = arrowPath.elementAt(e+1);
                    auto e3 = arrowPath.elementAt(e+2);
                    strokeOnly.cubicTo(e1, e2, e3);
                    e += 2;
                }
            }

            pathItem->setBrush(QBrush(Qt::black));
            QPen pen(Qt::black, 0.8);
            pen.setJoinStyle(Qt::RoundJoin);
            pen.setCapStyle(Qt::RoundCap);
            pathItem->setPen(pen);

            auto* lblItem = addText(lbl);
            lblItem->setDefaultTextColor(Qt::darkBlue);
            QFont f = lblItem->font();
            f.setPointSize(9);
            lblItem->setFont(f);
            lblItem->setPos(labelPos - QPointF(lblItem->boundingRect().width()/2,
                                               lblItem->boundingRect().height()/2));
        }
    }

    for (int i = 0; i < n; i++) {
        char label = states[i].label;
        QPointF pos = positions[label];

        auto* shadow = addEllipse(pos.x()+3, pos.y()+3,
                                  NODE_R*2, NODE_R*2,
                                  Qt::NoPen,
                                  QBrush(QColor(0,0,0,30)));
        shadow->setZValue(1);

        QColor fillColor(220, 232, 248);   // light blue
        QColor strokeColor(60, 110, 180);
        auto* circle = addEllipse(pos.x(), pos.y(), NODE_R*2, NODE_R*2,
                                  QPen(strokeColor, 1.8),
                                  QBrush(fillColor));
        circle->setZValue(2);

        auto* txt = addText(QString(label));
        QFont f = txt->font();
        f.setPointSize(14);
        f.setBold(true);
        txt->setFont(f);
        txt->setDefaultTextColor(QColor(30, 60, 120));
        txt->setPos(pos.x() + NODE_R - txt->boundingRect().width()/2,
                    pos.y() + NODE_R - txt->boundingRect().height()/2);
        txt->setZValue(3);

        auto* outTxt = addText(QString("/%1").arg(states[i].output));
        QFont sf = outTxt->font();
        sf.setPointSize(8);
        outTxt->setFont(sf);
        outTxt->setDefaultTextColor(QColor(80, 80, 80));
        outTxt->setPos(pos.x() + NODE_R - outTxt->boundingRect().width()/2,
                       pos.y() + NODE_R + 10);
        outTxt->setZValue(3);
    }

    QRectF br = itemsBoundingRect();
    setSceneRect(br.adjusted(-60, -60, 60, 60));
}

void DiagramScene::clear() {
    QGraphicsScene::clear();
}


DiagramView::DiagramView(const QString& title, QWidget* parent)
    : QWidget(parent), m_title(title)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    if (!title.isEmpty()) {
        auto* lbl = new QLabel(QString("<b>%1</b>").arg(title));
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("font-size: 13px; padding: 4px;");
        layout->addWidget(lbl);
    }

    auto* hint = new QLabel("Scroll to zoom · Drag to pan");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: palette(mid); font-size: 11px;");
    layout->addWidget(hint);

    m_scene = new DiagramScene(this);
    m_view  = new ZoomableView(m_scene, this);
    layout->addWidget(m_view);
}

void DiagramView::setMachine(const std::vector<State>& states) {
    m_scene->setMachine(states, m_title);
    m_view->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void DiagramView::clear() {
    m_scene->clear();
}
