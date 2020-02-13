#include "dialog.h"
#include "ui_dialog.h"
#include <qcustomplot.h>



void Dialog::realtimeData_and_to_File(double value)
{
    qDebug()<<value;
    static QTime time(QTime::currentTime());
    qDebug()<<time;
    double key = time.elapsed()/1000.0;// time elapsed since start of demo, in seconds

    write_to_file(key,value);

    static double lastPointKey = 0;
    if (key-lastPointKey > 0.002) // at most add point every 2 ms
    {

      plot->graph(0)->addData(key,value);
      lastPointKey = key;

    }
    plot->xAxis->setRange(key, 8, Qt::AlignCenter);
    plot->replot();

}

void Dialog::stopPlot()
{
    dataTimer->stop();
    port->close();
}

void Dialog::startPlot()
{
    port->open(QIODevice::ReadOnly);
    dataTimer->start(0);
}



Dialog::Dialog(QWidget *parent)
    : QDialog(parent)

{



                                    /* Serial Port setup*/


    port=new QSerialPort();
    is_port_active=false;
    //qDebug()<<QSerialPortInfo::availablePorts().count();

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasVendorIdentifier())
        {
            vendor=serialPortInfo.vendorIdentifier(); //search for vendor
        }

        if(serialPortInfo.hasProductIdentifier())
        {
            product=serialPortInfo.productIdentifier();// search for product
        }
    }


    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
        {
            if(serialPortInfo.productIdentifier()==product)
            {
                if(serialPortInfo.vendorIdentifier()==vendor) //product and vendor of port
                    PortName=serialPortInfo.portName();       // and device must coinside
                    is_port_active=true;
            }
        }
    }



    if(is_port_active==true)
    {

        port->setPortName(PortName);
        port->setBaudRate(QSerialPort::Baud115200);
        port->setDataBits(QSerialPort::Data8);
        port->setFlowControl(QSerialPort::NoFlowControl);
        port->setParity(QSerialPort::NoParity);
        port->setStopBits(QSerialPort::OneStop);

    }
    else
    {
        QMessageBox::warning(this,"Port error", "Cannot open port");
    }

                                /*Setting up plot layouts*/


    QWidget* window=new QWidget(this);
    window->setFixedSize(1200,900);
    QVBoxLayout* layout_left=new QVBoxLayout;
    QGridLayout* layout_right=new QGridLayout;
    QHBoxLayout* layout_main=new QHBoxLayout;



    plot=new QCustomPlot;
    plot->setFixedSize(QSize(900,800));




    start=new QPushButton("start plot");
    stop=new QPushButton("stop plot");
    info=new QPushButton("info");


    layout_left->addWidget(plot);
    layout_right->addWidget(start);
    layout_right->addWidget(stop);
    layout_right->addWidget(info);


    layout_main->addLayout(layout_left);
    layout_main->addLayout(layout_right);

    QFont font("Times");
    font.setPointSize(20);

    stop->setFixedSize(QSize(200,100));
    start->setFixedSize(QSize(200,100));
    info->setFixedSize(QSize(200,100));

    this->setStyleSheet("background-color: white;");


    QFile style(":/Res.css");
    style.open(QIODevice::ReadOnly);
    window->setStyleSheet(style.readAll());
    file.close();


    window->setLayout(layout_main); // add all layout to window







                                    /* Pen and Fonts Setup*/


    QPen* pen=new QPen();
    pen->setWidth(4);
    pen->setColor(Qt::red);
    pen->setStyle(Qt::SolidLine);

    font.setPointSize(15);


                                    /*File setup*/



    file.open(QIODevice::WriteOnly);
    file.setFileName("file.xml");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"Error";
    }


                                    /*Graph Setup*/

    plot->addGraph();
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    plot->xAxis->setTicker(timeTicker);
    plot->axisRect()->setupFullAxesBox();
    plot->yAxis->setRange(23, 27);
    plot->graph(0)->setPen(*pen);

    plot->yAxis->setLabel("T,°C");
    plot->yAxis->setLabelFont(font);
    plot->yAxis->setLabelColor(Qt::black);
    plot->yAxis->grid()->setSubGridVisible(true);
    plot->yAxis->ticker()->setTickCount(35);


    plot->xAxis->setLabel("t,sec");
    plot->xAxis->setLabelFont(font);
    plot->xAxis->setLabelColor(Qt::black);

    plot->xAxis2->setVisible(false);
    plot->setInteraction(QCP::iRangeZoom,true);
    dataTimer=new QTimer ();  // create  timer to plot real-time data


    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));



                                        /* Demo begin*/

    port->open(QSerialPort::ReadOnly);
    dataTimer->start(0);
    connect(dataTimer,&QTimer::timeout, this, &Dialog::read_data);
    connect(start,&QPushButton::clicked,this,&Dialog::startPlot);
    connect(stop,&QPushButton::clicked,this,&Dialog::stopPlot);
    connect(info,&QPushButton::clicked,this,&Dialog::info_message);

}


Dialog::~Dialog()
{
    if(port->isOpen())
       port->close();

    if(file.isOpen()==true)
       file.close();

}


void Dialog::read_data()
{
    port->waitForReadyRead(1000); //wait until data is available
    data=port->readAll();
    qDebug()<<data;
    int index_pos;
    index_pos=data.indexOf('\n');
    double tempValue = QString::fromLatin1(data.left(index_pos)).toDouble();
    qDebug()<<tempValue;
    realtimeData_and_to_File(tempValue);
}

void Dialog::write_to_file(double time, double value)
{
    QDomDocument document;
    QDomElement elem=document.createElement("Temperature");
    elem.setAttribute("Time",time);
    elem.setAttribute("Temp",value);
    document.appendChild(elem);


    QTextStream stream(&file);
    stream<<document.toString();

}

void Dialog::info_message()
{
    QMessageBox::information(this,"Info", "Use mouse to change range");
}


