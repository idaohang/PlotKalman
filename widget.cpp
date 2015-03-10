#include "widget.h"
#include "ui_widget.h"
#include <iostream>
#include <Eigen/Dense>
#include <random>
#include <iostream>
#include <ctime>
using Eigen::MatrixXd;

typedef std::tr1::ranlux64_base_01 Myeng;
typedef std::tr1::normal_distribution<double> Mydist;


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    ui->customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    this->plotKalman ();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::plotKalman()
{
    QVector<double> x(500), y0(500);

    //++++++++++++++++++++++++++init kalman  data+++++++++++++++++++++++

    MatrixXd Z(1,500);
    for(int i=0;i<500;i++){
        Z(0,i) = i+1;
    }

    MatrixXd noise(1,500); // [0, 100]

    Myeng eng;
    int myseed = 500 + (rand() % (int)(1000 - 500 + 1));
    std::cout<<"seed: "<< myseed <<std::endl;
    eng.seed(myseed);
    Mydist dist(0,1); //~N(0,1) distribution

    dist.reset(); // discard any cached values
    for (int i = 0; i < 500; i++)
    {
        noise(0,i) = dist(eng) ;
    }
    Z = Z + noise;

    MatrixXd X(2,1);
    X(0,0) = 0;
    X(1,0) = 0;
    MatrixXd P(2,2);
    P(0,0) = 1;
    P(0,1) = 0;
    P(1,0) = 0;
    P(1,1) = 1;
    MatrixXd F(2,2);
    F(0,0) = 1;
    F(0,1) = 1;
    F(1,0) = 0;
    F(1,1) = 1;
    MatrixXd Q(2,2);
    Q(0,0) = 0.0001;
    Q(0,1) = 0;
    Q(1,0) = 0;
    Q(1,1) = 0.0001;
    MatrixXd H(1,2);
    H(0,0) = 1;
    H(0,1) = 0;
    MatrixXd R(1,1) ;
    R(0,0)=1;

    MatrixXd X_(2,1);
    MatrixXd P_(2,2);
    MatrixXd K(2,1);
    for(int i = 0;i<500;i++){
        X_ = F*X;
        P_ =F*P*F.transpose() + Q;

        MatrixXd temp(0,0);
        temp= H*P_* (H.transpose()) + R;

        K =P_ * H.transpose() / temp(0,0);
        temp(0,0) = Z(i);
        X = X_ + K*(temp-H*X_);
        P = (MatrixXd::Identity(2,2) - K*H)*P_;
        x[i] = X(0);
        y0[i] = X(1);
    }

    //++++++++++++++++++++++++++++++ plot ++++++++++++++++++++++++++++++++

    ui->customPlot->graph(0)->setData(x, y0);
    ui->customPlot->graph(0)->rescaleAxes();
    ui->customPlot->replot ();

}

void Widget::on_pushButton_clicked()
{
    plotKalman();

}
