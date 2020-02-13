#ifndef DIALOG_H
#define DIALOG_H
#include <QTimer>
#include <QDialog>
#include <QTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <QIODevice>
#include <QFont>
#include <qcustomplot.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QtXml/QDomDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void write_to_file(double time, double value);

public slots:
    void realtimeData_and_to_File(double);
    void stopPlot();
    void startPlot();
    void read_data();
    void info_message();

private:
    QCustomPlot* plot;
    QTimer* dataTimer;
    QSerialPort* port;
    QString PortName;
    bool is_port_active;
    quint16 vendor;
    quint16 product;
    QByteArray data;
    QPushButton* start;
    QPushButton* stop;
    QPushButton* info;
    QFile file;
};
#endif // DIALOG_H
