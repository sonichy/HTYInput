#include "mainwindow.h"
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QRegExpValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("文本编辑器");
    setWindowIcon(QIcon(":/iconTE.png"));
    resize(400,300);
    pywidget = new QWidget;
    pywidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    pywidget->setWindowTitle("海天鹰输入法");
    pywidget->setWindowIcon(QIcon(":/icon.png"));
    QVBoxLayout *vbox = new QVBoxLayout;
    TE = new QTextEdit;
    setCentralWidget(TE);
    //vbox->addWidget(TE);
    pyedit = new QLineEdit;
    QRegExp RE("[A-Za-z/']+$");
    pyedit->setValidator(new QRegExpValidator(RE,this));
    vbox->addWidget(pyedit);
    label_hx = new QLabel;
    vbox->addWidget(label_hx);
    pywidget->setLayout(vbox);
    connect(pyedit,SIGNAL(textChanged(QString)),this,SLOT(search(QString)));

    pywidget->setLayout(vbox);
    qApp->installEventFilter(this);

    QSqlDatabase database;
    if(QSqlDatabase::contains("qt_sql_default_connection")){
        database = QSqlDatabase::database("qt_sql_default_connection");
    }else{
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("pyck.db");
    }
    if (!database.open()){
        qDebug() << "Error: Failed to connect database." << database.lastError();
    }else{

    }

}

MainWindow::~MainWindow()
{

}

void MainWindow::search(QString py)
{
    if(py==""){
        label_hx->setText("");
    }else{
        QSqlQuery query;
        yj.clear();
        yj = py.split("'");
        QString where = " where";
        for(int i=0;i<yj.size();i++){
            if(yj.at(i)!=""){
                where += " py like '" + yj.at(i) + "%'";
                if(i<yj.size()-1)where += " or";
            }
        }
        QString sql = "select hz from pyhz" + where;
        qDebug() << sql;
        if(!query.exec(sql)){
            qDebug() << query.lastError();
        }else{            
            hx.clear();
            while(query.next()){
                QString hz = query.value(0).toString();
                hx.append(hz);              
            }
            QString shx="";
            for(int i=0;i<hx.size();i++){
                shx += " " + QString::number(i) + "." + hx.at(i) + " ";
            }
            label_hx->setText(shx);
        }
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    QKeyEvent *KE = static_cast<QKeyEvent*>(event);

    if( obj == TE && event->type() == QEvent::KeyPress ){
        qDebug() << KE->text();
        if( KE->key() >= Qt::Key_A && KE->key() <= Qt::Key_Z){
            QTextCursor c = TE->textCursor();
            pywidget->move(TE->viewport()->mapToGlobal(TE->cursorRect(c).bottomLeft()) + QPoint(0,5));
            pywidget->show();
        }
    }
    if( obj == pyedit && event->type() == QEvent::KeyPress )
    {
        qDebug() << KE->text();
        if( KE->key() >= Qt::Key_0 && KE->key() <= Qt::Key_9 ){
            if( pyedit->text().lastIndexOf("'") != -1 ){
                pyedit->setText(pyedit->text().replace(yj.at(0), hx.at(KE->key() - Qt::Key_0)));
            }else{
                if( (KE->key() - Qt::Key_0 ) < hx.size() ){
                    TE->insertPlainText(hx.at(KE->key() - Qt::Key_0));
                    pyedit->setText("");
                    pywidget->hide();
                }
            }
        }
        if( KE->key()==Qt::Key_Space ){
            if( hx.size() >0 ){
                TE->insertPlainText(hx.at(0));
                pyedit->setText("");
                pywidget->hide();
            }
        }
        if( KE->key()<Qt::Key_A && KE->key()>Qt::Key_Z){
            TE->insertPlainText(KE->text());
        }
        if( KE->key() == Qt::Key_Return && KE->key() == Qt::Key_Enter){
            TE->insertPlainText(pyedit->text());
            pyedit->setText("");
            pywidget->hide();
        }
        if( KE->key()==Qt::Key_Escape ){
            pyedit->setText("");
            pywidget->hide();
        }
    }
    return QObject::eventFilter(obj, event);
}
