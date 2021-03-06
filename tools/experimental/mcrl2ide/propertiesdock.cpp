// Author(s): Olav Bunte
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "propertiesdock.h"

#include <QMainWindow>
#include <QLabel>

PropertiesDock::PropertiesDock(ProcessSystem* processSystem,
                               FileSystem* fileSystem, QWidget* parent)
    : QDockWidget("Properties", parent)
{
  this->processSystem = processSystem;
  this->fileSystem = fileSystem;
  previousHorizontalScrollbarMaximum = 0;

  /* create the properties layout */
  propertiesLayout = new QVBoxLayout();
  propertiesLayout->setAlignment(Qt::AlignTop);
  setToNoProperties();

  QWidget* innerScrollWidget = new QWidget();
  innerScrollWidget->setLayout(propertiesLayout);

  innerDockWidget = new QScrollArea();
  innerDockWidget->setWidget(innerScrollWidget);
  innerDockWidget->setWidgetResizable(true);
  innerDockWidget->setStyleSheet("QScrollArea { border: none; }");

  hScrollBar = innerDockWidget->horizontalScrollBar();
  connect(hScrollBar, SIGNAL(rangeChanged(int, int)), this,
          SLOT(keepContentsLeft(int, int)));

  this->setWidget(innerDockWidget);
}

void PropertiesDock::setToNoProperties()
{
  /* empty the layout (is usually already empty) */
  QLayoutItem* item;
  while ((item = propertiesLayout->takeAt(0)))
  {
    propertiesLayout->removeWidget(item->widget());
    delete item->widget();
  }
  /* show a QLabel that tells the user that no properties have been defined */
  QLabel* noPropertiesLabel = new QLabel("No properties have been defined");
  propertiesLayout->addWidget(noPropertiesLabel);

  propertyWidgets.clear();
}

void PropertiesDock::keepContentsLeft(int min, int max)
{
  Q_UNUSED(min);
  if (max > previousHorizontalScrollbarMaximum)
  {
    hScrollBar->setValue(hScrollBar->value() +
                         (max - previousHorizontalScrollbarMaximum));
  }
  previousHorizontalScrollbarMaximum = max;
}

void PropertiesDock::addProperty(Property* property)
{
  if (propertyWidgets.empty())
  {
    /* remove the QLabel */
    QWidget* label = propertiesLayout->takeAt(0)->widget();
    propertiesLayout->removeWidget(label);
    delete label;
    propertiesLayout->addStretch(1);
  }

  /* add the property to the rest */
  PropertyWidget* propertyWidget =
      new PropertyWidget(property, processSystem, fileSystem, this);
  propertiesLayout->insertWidget(propertiesLayout->count() - 1, propertyWidget);
  propertyWidgets.push_back(propertyWidget);
}

void PropertiesDock::deleteProperty(PropertyWidget* propertyWidget)
{
  propertiesLayout->removeWidget(propertyWidget);
  if (propertiesLayout->isEmpty())
  {
    this->setToNoProperties();
  }
  propertyWidgets.remove(propertyWidget);
}

void PropertiesDock::verifyAllProperties()
{
  for (PropertyWidget* propertyWidget : propertyWidgets)
  {
    propertyWidget->actionVerify();
  }
}

void PropertiesDock::resetAllPropertyWidgets()
{
  for (PropertyWidget* propertyWidget : propertyWidgets)
  {
    propertyWidget->resetWidget();
  }
}
