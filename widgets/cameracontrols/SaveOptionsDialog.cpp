//
// Created by lab on 2021/9/20.
//

#include "SaveOptionsDialog.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>

SaveOptionsDialog::SaveOptionsDialog(const SaveOptions &op)
    : QDialog(), saveDV(new QCheckBox(tr("保存DV图片"))),
      saveDVSImg(new QCheckBox(tr("保存DVS图片"))),
      saveDVSRaw(new QCheckBox(tr("保存DVS RAW文件"))),
      savePathEdit(new QLineEdit),
      browseButton(new QPushButton(tr("浏览..."))),
      options(op) {
  auto layout = new QFormLayout;
  //    saveDV=new QCheckBox(tr("保存DV图片"));
  layout->addRow(saveDV);
  layout->addRow(saveDVSImg);
  layout->addRow(saveDVSRaw);
  
  // 添加保存路径行
  auto pathLayout = new QHBoxLayout;
  savePathEdit->setText(options.savePath);
  pathLayout->addWidget(savePathEdit);
  pathLayout->addWidget(browseButton);
  
  layout->addRow(tr("保存路径:"), pathLayout);
  
  if (options.saveDVSRaw) {
    saveDVSRaw->setCheckState(Qt::Checked);
  }
  if (options.saveDV) {
    saveDV->setCheckState(Qt::Checked);
  }
  if (options.saveDVSImg) {
    saveDVSImg->setCheckState(Qt::Checked);
  }

  connect(saveDV, &QCheckBox::stateChanged, this,
          [this](int s) { options.saveDV = (s == Qt::Checked); });
  connect(saveDVSImg, &QCheckBox::stateChanged, this,
          [this](int s) { options.saveDVSImg = (s == Qt::Checked); });
  connect(saveDVSRaw, &QCheckBox::stateChanged, this,
          [this](int s) { options.saveDVSRaw = (s == Qt::Checked); });
          
  // 连接保存路径编辑框和浏览按钮
  connect(savePathEdit, &QLineEdit::textChanged, this,
          [this](const QString &text) { options.savePath = text; });
  connect(browseButton, &QPushButton::clicked, this, &SaveOptionsDialog::browseForSavePath);
          
  auto buttons = new QDialogButtonBox;
  buttons->addButton(QDialogButtonBox::StandardButton::Cancel);
  buttons->addButton(QDialogButtonBox::StandardButton::Ok);
  connect(buttons, &QDialogButtonBox::accepted, this,
          &SaveOptionsDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this,
          &SaveOptionsDialog::reject);
  layout->addRow(buttons);
  setLayout(layout);

  setModal(true);
}

void SaveOptionsDialog::browseForSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择保存路径"),
                                                    options.savePath,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        savePathEdit->setText(dir);
        options.savePath = dir;
    }
}
