#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QWidget *pywidget;
    QLineEdit *pyedit;
    QLabel *label_hx;
    QStringList hx,yj;
    QTextEdit *TE;

private slots:
    void search(QString);
    //void choosed(int);
};

#endif // MAINWINDOW_H
