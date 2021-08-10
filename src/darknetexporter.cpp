#include "darknetexporter.h"

void DarknetExporter::generateLabelIds(const QString names_file){
    id_map.clear();

    // Force sort list
    QStringList class_list;
    QFile fh(names_file);

    if (fh.open(QIODevice::ReadOnly)) {

        while (!fh.atEnd()) {
            // Darknet name file is just a newline delimited list of classes
            QByteArray line = fh.readLine();
            class_list.append(line);
        }
    }

    if(class_list.size() == 0){
        qDebug() << "No classes found in names file.";
        return;
    }

    int i = 0;
    for(auto &name : class_list){
        auto cleaned_name = name.simplified().toLower();
        id_map[cleaned_name] = i++;
        qDebug() << "Adding: " << cleaned_name << " (" << i << ")";
    }
}

double clamp(double val, double min, double max){
    if(val < min) val = min;
    if(val > max) val = max;
    return val;
}

void DarknetExporter::writeLabels(const cv::Mat &image, const QString label_filename, const QList<BoundingBox> labels){

    // Still make a label file even if there are no detections. This is important
    // for background class detection.

    QFile f(label_filename);

    // Delete existing files for simplicity.
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        BoundingBox label;
        foreach(label, labels){
            QString text;

            // Check if this label exists in the database
            if(id_map.find(label.classname.toLower()) == id_map.end()){
                qDebug() << "Couldn't find this label in the names file: " << label.classname.toLower();
                continue;
            }

            double x = clamp(static_cast<double>(label.rect.center().x())/image.cols, 0.0, 0.999);
            double y = clamp(static_cast<double>(label.rect.center().y())/image.rows, 0.0, 0.999);
            double width = clamp(static_cast<double>(label.rect.width())/image.cols, 0.0, 0.999);
            double height = clamp(static_cast<double>(label.rect.height())/image.rows, 0.0, 0.999);

            text += QString("%1").arg(id_map[label.classname.toLower()]);
            text += QString(" %1").arg(x);
            text += QString(" %1").arg(y);
            text += QString(" %1").arg(width);
            text += QString(" %1").arg(height);
            text += "\n";

            f.write(text.toUtf8());

        }
    }
}

bool DarknetExporter::processImages(const QString folder, const QList<QString> images, export_image_type split_type){

    QString image_path;
    QList<BoundingBox> labels;

    QProgressDialog progress("...", "Abort", 0, images.size(), static_cast<QWidget*>(parent()));
    progress.setWindowModality(Qt::WindowModal);

    QString split_text = "";
    if(split_type == EXPORT_VAL){
        split_text = "VAL";
        progress.setWindowTitle("Exporting validation images");
    }else if(split_type == EXPORT_TRAIN){
        split_text = "TRAIN";
        progress.setWindowTitle("Exporting train images");
    }else if(split_type == EXPORT_TEST){
        split_text = "TEST";
        progress.setWindowTitle("Exporting test images");
    }else{
        split_text = "UNASSIGNED";
    }

    int i = 0;

    foreach(image_path, images){

        if(progress.wasCanceled()){
            break;
        }

        qDebug() << image_path;
        project->getLabels(image_path, labels);

        if(!export_unlabelled && labels.size() == 0) continue;

        QString extension = QFileInfo(image_path).suffix();
        QString filename_noext = QFileInfo(image_path).completeBaseName();
        QString image_filename = QString("%1/%2%3.%4").arg(folder).arg(filename_prefix).arg(filename_noext).arg(extension);

        // Correct for duplicate file names in output
        int dupe_file = 1;
        while(QFile(image_filename).exists()){
            image_filename = QString("%1/%2%3_%4.%5")
                                    .arg(folder)
                                    .arg(filename_prefix)
                                    .arg(filename_noext)
                                    .arg(dupe_file++)
                                    .arg(extension);
        }

        cv::Mat image = cv::imread(image_path.toStdString());
        //saveImage(image, image_filename);

        QFile::copy(image_path, image_filename);

        QString label_filename = QString("%1/%2.txt").arg(folder).arg(filename_noext);
        writeLabels(image, label_filename, labels);

        progress.setValue(i++);
        progress.setLabelText(image_filename);

    }

    return true;
}

void DarknetExporter::process(){
    processImages(train_folder, train_set, EXPORT_TRAIN);
    processImages(val_folder, validation_set, EXPORT_VAL);
}
