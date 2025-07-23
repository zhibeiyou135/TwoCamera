//
// Created by lab on 2021/9/20.
//

#ifndef TWOCAMERA_SAVEOPTIONSDIALOG_H
#define TWOCAMERA_SAVEOPTIONSDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class SaveOptionsDialog : public QDialog {
  Q_OBJECT
public:
  struct SaveOptions {
    bool saveDV = false;
    bool saveDVSImg = false;
    bool saveDVSRaw = true;  // 保存DVS RAW格式文件（以前是BIN格式）
    QString savePath = "recordings"; // 默认保存路径
  };

  SaveOptionsDialog(const SaveOptions &op);

  SaveOptions getConfig() { return options; };

private slots:
  void browseForSavePath();

private:
  QCheckBox *saveDV, *saveDVSImg, *saveDVSRaw;  // 修改为saveDVSRaw
  QLineEdit *savePathEdit;
  QPushButton *browseButton;
  SaveOptions options;
};

#endif // TWOCAMERA_SAVEOPTIONSDIALOG_H
