//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbitsamplingreport.h"

#include <cassert>

#include "SamplingReport.h"
#include "orbitdataviewpanel.h"
#include "orbittreeview.h"
#include "ui_orbitsamplingreport.h"

//-----------------------------------------------------------------------------
OrbitSamplingReport::OrbitSamplingReport(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitSamplingReport) {
  ui->setupUi(this);
  if (!m_SamplingReport || !m_SamplingReport->HasCallstacks()) {
    ui->NextCallstackButton->setEnabled(false);
    ui->NextCallstackButton->setStyleSheet(
        QString::fromUtf8("QPushButton:disabled{ color: gray }"));
    ui->PreviousCallstackButton->setEnabled(false);
    ui->PreviousCallstackButton->setStyleSheet(
        QString::fromUtf8("QPushButton:disabled{ color: gray }"));
  }

  QList<int> sizes;
  sizes.append(5000);
  sizes.append(5000);
  ui->splitter->setSizes(sizes);
}

//-----------------------------------------------------------------------------
OrbitSamplingReport::~OrbitSamplingReport() { delete ui; }

//-----------------------------------------------------------------------------
void OrbitSamplingReport::Initialize(DataView* callstack_data_view,
                                     std::shared_ptr<SamplingReport> report) {
  ui->CallstackTreeView->Initialize(
      callstack_data_view, SelectionType::kExtended, FontType::kDefault, false);
  m_SamplingReport = report;

  if (!report) return;

  m_SamplingReport->SetUiRefreshFunc([&]() { this->Refresh(); });

  for (SamplingReportDataView& report_data_view : report->GetThreadReports()) {
    QWidget* tab = new QWidget();
    tab->setObjectName(QStringLiteral("tab"));

    QGridLayout* gridLayout_2 = new QGridLayout(tab);
    gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
    OrbitDataViewPanel* treeView = new OrbitDataViewPanel(tab);
    treeView->SetDataModel(&report_data_view);

    if (!report_data_view.IsSortingAllowed()) {
      treeView->GetTreeView()->setSortingEnabled(false);
    } else {
      int column = report_data_view.GetDefaultSortingColumn();
      Qt::SortOrder order =
          report_data_view.GetColumns()[column].initial_order ==
                  DataView::SortingOrder::Ascending
              ? Qt::AscendingOrder
              : Qt::DescendingOrder;
      treeView->GetTreeView()->sortByColumn(column, order);
    }

    treeView->setObjectName(QStringLiteral("treeView"));
    gridLayout_2->addWidget(treeView, 0, 0, 1, 1);
    treeView->GetTreeView()->setSelectionMode(OrbitTreeView::ExtendedSelection);
    treeView->GetTreeView()->header()->resizeSections(
        QHeaderView::ResizeToContents);

    treeView->Link(ui->CallstackTreeView);

    QString threadName = QString::fromStdString(report_data_view.GetName());
    ui->tabWidget->addTab(tab, threadName);
  }
}

//-----------------------------------------------------------------------------
void OrbitSamplingReport::on_NextCallstackButton_clicked() {
  assert(m_SamplingReport);
  m_SamplingReport->IncrementCallstackIndex();
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitSamplingReport::on_PreviousCallstackButton_clicked() {
  assert(m_SamplingReport);
  m_SamplingReport->DecrementCallstackIndex();
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitSamplingReport::Refresh() {
  assert(m_SamplingReport);
  if (m_SamplingReport->HasCallstacks()) {
    ui->NextCallstackButton->setEnabled(true);
    ui->PreviousCallstackButton->setEnabled(true);
  }
  std::string label = m_SamplingReport->GetSelectedCallstackString();
  ui->CallStackLabel->setText(QString::fromStdString(label));
  ui->CallstackTreeView->Refresh();
}
