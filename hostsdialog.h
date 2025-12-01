#ifndef HOSTSDIALOG_H
#define HOSTSDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class HostsDialog;
}

class HostsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HostsDialog(const QStringList &hosts, QWidget *parent = nullptr);
    ~HostsDialog();

    QStringList hosts() const;

private Q_SLOTS:
    void onAddHost();
    void onRemoveHost();
    void onSelectionChanged();

private:
    Ui::HostsDialog *ui;
};

#endif // HOSTSDIALOG_H
