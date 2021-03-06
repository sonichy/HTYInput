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
#include <QString>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    isPinYin = false;
    setWindowTitle("文本编辑器");
    setWindowIcon(QIcon(":/iconTE.png"));
    resize(400,300);
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(switchPinYin()));
    pywidget = new QWidget;
    pywidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    pywidget->setWindowTitle("海天鹰输入法");
    pywidget->setWindowIcon(QIcon(":/icon.png"));
    QVBoxLayout *vbox = new QVBoxLayout;
    TE = new QTextEdit;
    setCentralWidget(TE);    
    pyedit = new QLineEdit;
    QRegExp RE("[A-Za-z/']+$");
    pyedit->setValidator(new QRegExpValidator(RE,pyedit));
    vbox->addWidget(pyedit);
    label_hx = new QLabel;
    vbox->addWidget(label_hx);
    pywidget->setLayout(vbox);
    connect(pyedit,SIGNAL(textChanged(QString)),this,SLOT(search(QString)));

    pywidget->setLayout(vbox);
    qApp->installEventFilter(this);

    QSqlDatabase database;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        database = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("chinese.db");
    }
    if (!database.open()){
        qDebug() << "Error: Failed to connect database." << database.lastError();
    } else {

    }

}

MainWindow::~MainWindow()
{

}

void MainWindow::search(QString py)
{
    if (py == "") {
        label_hx->setText("");
    } else {
        QSqlQuery query;
        yj.clear();
        yj = py.split("'");
        QString where = " where";
        for (int i=0; i<yj.size(); i++) {
            if (yj.at(i) != "" && !yj.at(i).contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
                //where += " pinyin like '" + yj.at(i) + "%'";
                where += " pinyin='" + yj.at(i) + "'";
                if (i < yj.size()-1) where += " or";
            }
        }
        if (where != " where") {
            QString sql = "select chinese from singlePinyin" + where;
            qDebug() << sql;
            if (!query.exec(sql)) {
                qDebug() << query.lastError();
            } else {
                hx.clear();
                while (query.next()) {
                    QString hz = query.value(0).toString();
                    //qDebug() << hz;
                    //hx.append(hz);
                    hx = hz.split(" ",QString::SkipEmptyParts);
                }
                QString shx="";
                int max;
                page = 1;
                if(hx.size()<10){
                    max = hx.size();
                }else{
                    max = 10;
                }
                for (int i=0; i<max; i++) {
                    shx += " " + QString::number(i) + "." + hx.at(i) + " ";
                }
                label_hx->setText(shx);
            }
        }
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    //qDebug() << isPinYin;
    if(isPinYin){
        QKeyEvent *KE = static_cast<QKeyEvent*>(event);

        if( obj == TE && event->type() == QEvent::KeyPress ){
            qDebug() << "TextEdit" << KE->text();
            if( KE->key() >= Qt::Key_A && KE->key() <= Qt::Key_Z){
                QTextCursor c = TE->textCursor();
                pywidget->move(TE->viewport()->mapToGlobal(TE->cursorRect(c).bottomLeft()) + QPoint(0,5));
                pywidget->show();
                pyedit->setText(KE->text());
                return true;
            }
        }

        if (obj == pyedit && event->type() == QEvent::KeyPress) {
            //qDebug() << "LineEdit" << KE->text();
            int yjc = 0;
            for (int i=0; i<yj.size(); i++) {
                if (!yj.at(i).contains(QRegExp("[\\x4e00-\\x9fa5]+"))) { // 不含中文
                    yjc = i;
                    break;
                }
            }

            if (KE->key() >= Qt::Key_0 && KE->key() <= Qt::Key_9) {
                qDebug() << "选" << KE->key() - Qt::Key_0 << "候选个数" << hx.size();
                if (pyedit->text().lastIndexOf("'") != -1 ) {
                    pyedit->setText(pyedit->text().replace(yj.at(yjc), hx.at(KE->key() - Qt::Key_0 + 10*(page-1))));
                } else {
                    if ((KE->key() - Qt::Key_0 ) < hx.size()) {
                        TE->insertPlainText(hx.at(KE->key() - Qt::Key_0 + 10*(page-1)));
                        pyedit->setText("");
                        pywidget->hide();
                    }
                }
                return false;
            }

            if (KE->key() == Qt::Key_Space) {
                if( hx.size() == 0 ){
                    TE->insertPlainText(pyedit->text());
                    pyedit->setText("");
                    pywidget->hide();
                }
            }

            if (KE->key() < Qt::Key_A && KE->key() > Qt::Key_Z) {
                TE->insertPlainText(KE->text());
            }

            if (KE->key() == Qt::Key_Return || KE->key() == Qt::Key_Enter) {
                TE->insertPlainText(pyedit->text());
                pyedit->setText("");
                pywidget->hide();
            }

            if (KE->key() == Qt::Key_Escape) {
                pyedit->setText("");
                pywidget->hide();
            }

            if(KE->key() == Qt::Key_Comma){
                //qDebug() << "page" << page;
                if(page>1){
                    page--;
                    int min,max;
                    min = 10*(page-1);
                    if(hx.size()<10*page){
                        max = hx.size();
                    }else{
                        max = 10*page;
                    }
                    //qDebug() << "page" << page << min << max;
                    int id = 0;
                    QString shx = "";
                    for (int i=10*(page-1); i<max; i++) {
                        shx += " " + QString::number(id) + "." + hx.at(i) + " ";
                        id++;
                    }
                    label_hx->setText(shx);
                }
            }

            if(KE->key() == Qt::Key_Period){
                //qDebug() << "page" << page;
                if(hx.size()>10*page){
                    page++;
                    int min,max;
                    min = 10*(page-1);
                    if(hx.size()<10*page){
                        max = hx.size();
                    }else{
                        max = 10*page;
                    }
                    //qDebug() << "page" << page << min << max;
                    int id = 0;
                    QString shx = "";
                    for (int i=10*(page-1); i<max; i++) {
                        shx += " " + QString::number(id) + "." + hx.at(i) + " ";
                        id++;
                    }
                    label_hx->setText(shx);
                }
            }


        }
        return QObject::eventFilter(obj, event);
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void MainWindow::switchPinYin()
{
    isPinYin = !isPinYin;
}
