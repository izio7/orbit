//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbitdataviewpanel.h"

#include <utility>

#include "ui_orbitdataviewpanel.h"

//-----------------------------------------------------------------------------
OrbitDataViewPanel::OrbitDataViewPanel(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitDataViewPanel) {
  ui->setupUi(this);
  ui->label->hide();
}

//-----------------------------------------------------------------------------
OrbitDataViewPanel::~OrbitDataViewPanel() { delete ui; }

//-----------------------------------------------------------------------------
void OrbitDataViewPanel::Initialize(DataView* data_view,
                                    SelectionType selection_type,
                                    FontType font_type, bool is_main_instance) {
  ui->treeView->Initialize(data_view, selection_type, font_type);

  if (is_main_instance) {
    ui->treeView->GetModel()->GetDataView()->SetAsMainInstance();
  }

  std::string label = ui->treeView->GetLabel();
  if (!label.empty()) {
    this->ui->label->setText(QString::fromStdString(label));
    this->ui->label->show();
  }
}

//-----------------------------------------------------------------------------
OrbitTreeView* OrbitDataViewPanel::GetTreeView() { return ui->treeView; }

//-----------------------------------------------------------------------------
void OrbitDataViewPanel::Link(OrbitDataViewPanel* a_Panel) {
  ui->treeView->Link(a_Panel->ui->treeView);
}

//-----------------------------------------------------------------------------
void OrbitDataViewPanel::Refresh() { ui->treeView->Refresh(); }

//-----------------------------------------------------------------------------
void OrbitDataViewPanel::SetDataModel(DataView* model) {
  ui->treeView->SetDataModel(model);
}

//-----------------------------------------------------------------------------
void OrbitDataViewPanel::SetFilter(const QString& a_Filter) {
  ui->FilterLineEdit->setText(a_Filter);
  ui->treeView->OnFilter(a_Filter);
}

//-----------------------------------------------------------------------------
void OrbitDataViewPanel::Select(int a_Row) { ui->treeView->Select(a_Row); }

//-----------------------------------------------------------------------------
void OrbitDataViewPanel::on_FilterLineEdit_textEdited(const QString& a_Text) {
  ui->treeView->OnFilter(a_Text);
}
