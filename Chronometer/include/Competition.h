#ifndef COMPETITION_H
#define COMPETITION_H

#include <QObject>
#include <map>
#include <vector>

class CCompetition : public QObject {
  Q_OBJECT

private:


public:
  CCompetition(QObject* parent=nullptr);
  CCompetition(const QString& file_path);
  virtual ~CCompetition();

private slots:
public slots:
signals:

};

#endif // COMPETITION_H
