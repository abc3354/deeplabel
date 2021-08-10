#ifndef PASCALVOCEXPORTER_H
#define PASCALVOCEXPORTER_H

#include <baseexporter.h>
#include <QObject>
#include <QImageReader>
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <boundingbox.h>
#include <opencv2/opencv.hpp>
#include <labelproject.h>
#include <random>

class PascalVocExporter : public BaseExporter
{
    Q_OBJECT
public:
    explicit PascalVocExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}

signals:
    void export_progress(int);

public slots:
    void splitData(float split=1, bool shuffle=false, int seed=42);
    bool setOutputFolder(QString folder);
    void setExportUnlabelled(bool res){export_unlabelled = res;}
    void setExportMap(bool map){export_map = map;}
    void process();

private:
    LabelProject *project;
    QList<QString> train_set;
    QList<QString> validation_set;
    QList<QString> images;

    QString train_folder;
    QString train_label_folder;
    QString train_image_folder;

    QString val_folder;
    QString val_label_folder;
    QString val_image_folder;

    QString output_folder;
    bool export_unlabelled = false;
    bool export_map;


    std::map<QString, int> id_map;

    void writeLabels(const cv::Mat &image, const QString image_filename, const QString label_filename, const QList<BoundingBox> labels);
    bool processImages(const QString folder, const QList<QString> images);
    void saveLabelMap(QString filename);
    bool saveImage(cv::Mat &image, const QString output, const double scale_x = -1.0, const double scale_y = -1.0);
};

#endif // PASCALVOCEXPORTER_H
