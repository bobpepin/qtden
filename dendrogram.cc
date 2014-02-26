#include <algorithm>
#include <cmath>
#include <sstream>

#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPrinter>
#include <QPrintDialog>

#include "dendrogram.h"
#include "vectorplot.h"

Dendrogram::Dendrogram(esom::LabelTree& tree, Dataset& dataset, 
		       QWidget *parent) 
  : QWidget(parent), tree(tree), dataset(dataset), activeItem(-1), 
    itemOrder(dataset.inputs())
{ 
  buildScene(scene);
  QGraphicsView *view = new QGraphicsView(&scene, this);
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(view);
  this->setLayout(layout);
  setFocusPolicy(Qt::StrongFocus);

  view->show();
}

void Dendrogram::buildLabelItems() {
  int maxWidth = 0;
  for(unsigned int i=0; i < dataset.inputs(); i++) {
    std::stringstream ss;
    ss << '[' << dataset.row_labels.id(i) << ']' << ' ' << dataset.row_labels.name(i);
    QString label(ss.str().c_str());
    VectorLabelItem *text = new VectorLabelItem(i, label, this);
    labelItems.push_back(text);
    QRectF bb = text->boundingRect();
    if(bb.width() > maxWidth) {
      maxWidth = std::ceil(bb.width());
    }
  }
  labelWidth = maxWidth;
}

void Dendrogram::buildScene(QGraphicsScene& scene) {
  QPointF p;

  buildLabelItems();
  //  std::cerr << "labelWidth = " << labelWidth << std::endl;
  buildNode(scene, 0, tree.top(), p);

  // std::cerr << "itemOrder[" << itemOrder.size() << "] =";
  // for(unsigned int i=0; i < itemOrder.size(); i++) {
  //   std::cerr << ' ' << dataset.row_labels.id(itemOrder[i]);
  // }
  // std::cerr << std::endl;

  QPointF p2(p.x() + EDGE_LENGTH, p.y());
  scene.addLine(QLineF(p, p2));
}

int Dendrogram::buildNode(QGraphicsScene& scene, int pos, int node, 
			  QPointF& snap) {
  if(tree.isLeaf(node)) {
    // QString label(dataset.rowName(tree.leafValue(node)).c_str());
    // VectorLabelItem *text = new VectorLabelItem(dataset, tree.leafValue(node), label);
    //    std::cerr << "leafValue(" << node << ") = " << tree.leafValue(node) << std::endl;
    VectorLabelItem *text = labelItems[tree.leafValue(node)];
    QString label = text->text();
    text->setText(label + " " + QString::number(labelItems.size() - 1 - pos));
    itemOrder[pos] = tree.leafValue(node);
    scene.addItem(text);
    text->setPos(10 + labelWidth - text->boundingRect().width(), 50+20*pos);
    snap.setX(labelWidth + 10);
    snap.setY(58+20*pos);
    return 1;
  }
  QPointF snapLeft, snapRight;
  int right_count = buildNode(scene, pos, tree.right(node), 
			      snapRight);
  int left_count = buildNode(scene, pos+right_count, tree.left(node), snapLeft);
  double cy = snapLeft.y() + (snapRight.y() - snapLeft.y())/2;
  double cx = std::max(snapLeft.x(), snapRight.x()) + EDGE_LENGTH;
  QPointF center(cx, cy);
  QPointF leftEnd(cx, snapLeft.y());
  QPointF rightEnd(cx, snapRight.y());
  scene.addLine(QLineF(leftEnd, rightEnd));
  scene.addLine(QLineF(snapLeft, leftEnd));
  scene.addLine(QLineF(snapRight, rightEnd));
  snap.setX(cx);
  snap.setY(cy);
  //  painter.drawLine(center, snap);
  return left_count + right_count;
}

void Dendrogram::print() {
  QPrinter printer;
  if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);
    scene.render(&painter);
  }
}

void Dendrogram::activateItem(int value) {
  if(activeItem >= 0) {
    labelItems[activeItem]->lowlight();
  }
  labelItems[value]->highlight();
  activeItem = value;
  emit select(value);
}

void Dendrogram::keyPressEvent(QKeyEvent *event) {
  int key = event->key();
  if(key != Qt::Key_Up && key != Qt::Key_Down) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(activeItem == -1) {
    activateItem(itemOrder[0]);
    return;
  }
  std::vector<int>::const_iterator iter;
  iter = std::find(itemOrder.begin(), itemOrder.end(), activeItem);
  assert(iter != itemOrder.end());
  if(key == Qt::Key_Up) {
    if(iter != itemOrder.begin()) {
      iter--;
      activateItem(*iter);
    }
  } else if(key == Qt::Key_Down) {
    iter++;
    if(iter != itemOrder.end()) {
      activateItem(*iter);
    }
  }
}

VectorLabelItem::VectorLabelItem(int row, QString& text, 
				 Dendrogram *dendrogram, QGraphicsItem *parent) 
  : QGraphicsSimpleTextItem(text, parent), dendrogram(dendrogram), row(row) {
}

void VectorLabelItem::mousePressEvent(QGraphicsSceneMouseEvent*) {
  dendrogram->activateItem(row);
  //  std::cerr << "mouse" << row << std::endl;
}

void VectorLabelItem::highlight() {
  QBrush b = brush();
  b.setColor(QColor(0, 0, 255));
  setBrush(b);
  update();
}

void VectorLabelItem::lowlight() {
  QBrush b = brush();
  b.setColor(QColor(0, 0, 0));
  setBrush(b);
  update();
}
