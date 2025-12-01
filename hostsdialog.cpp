#include "hostsdialog.h"
#include "ui_hostsdialog.h"

HostsDialog::HostsDialog(const QStringList &hosts, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HostsDialog)
{
    ui->setupUi(this);

    // Populate list
    for (const QString &host : hosts)
    {
        ui->listHosts->addItem(host);
    }

    ui->btnRemoveHost->setEnabled(false);

    connect(ui->btnAddHost, &QPushButton::clicked, this,
            &HostsDialog::onAddHost);
    connect(ui->btnRemoveHost, &QPushButton::clicked, this,
            &HostsDialog::onRemoveHost);
    connect(ui->listHosts, &QListWidget::itemSelectionChanged, this,
            &HostsDialog::onSelectionChanged);
    connect(ui->btnClose, &QPushButton::clicked, this,
            &HostsDialog::accept);

}

HostsDialog::~HostsDialog()
{
    delete ui;
}

QStringList HostsDialog::hosts() const
{
    QStringList result;

    for (int i = 0; i < ui->listHosts->count(); i++)
    {
        result << ui->listHosts->item(i)->text();
    }

    return result;
}

void HostsDialog::onAddHost()
{
    QString host = ui->editNewHost->text().trimmed();
    if (host.isEmpty())
        return;

    // Ignore duplicates
    QList<QListWidgetItem*> matches = ui->listHosts->findItems(host,
        Qt::MatchExactly);
    if (!matches.isEmpty())
    {
        ui->editNewHost->clear();
        return;
    }

    ui->listHosts->addItem(host);
    ui->editNewHost->clear();
}

void HostsDialog::onRemoveHost()
{
    QListWidgetItem *item = ui->listHosts->currentItem();

    if (!item)
        return;

    delete item;
}

void HostsDialog::onSelectionChanged()
{
    ui->btnRemoveHost->setEnabled(ui->listHosts->currentItem() != nullptr);
}


