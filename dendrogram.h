#include <vector>
#include <string>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>

#include <esom.h>

#include "dataset.h"

class Dendrogram;

class VectorLabelItem : public QGraphicsSimpleTextItem {
private:
  Dendrogram *dendrogram;
  int row;
public:
  VectorLabelItem(int, QString&, Dendrogram*, QGraphicsItem *parent=0);
  void highlight();
  void lowlight();

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

class Dendrogram : public QWidget { 
  Q_OBJECT

 private:
  static const unsigned int EDGE_LENGTH = 5;
  esom::LabelTree& tree;
  Dataset& dataset;
  std::vector<VectorLabelItem*> labelItems;
  int labelWidth;
  QGraphicsScene scene;
  int activeItem;
  std::vector<int> itemOrder;

  int paintNode(QPainter&, int, int, QPoint&);
  void buildLabelItems();
  void buildScene(QGraphicsScene& scene);
  int buildNode(QGraphicsScene& scene, int, int, QPointF&);
 protected:
  void keyPressEvent(QKeyEvent*);
  
 public:
  Dendrogram(esom::LabelTree& tree, Dataset& dataset, 
	     QWidget *parent=0);

 public slots:
  void activateItem(int);
  void print();

 signals:
  void select(int);
};
