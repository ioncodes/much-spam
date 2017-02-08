#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <iostream>

void checkFiles(QString path);
void createFile(QString path);
void createTree(QString path);
void checkTree(QString path);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList args = QCoreApplication::arguments();
    if(args.count() != 3)
        return 0;
    QString mode = args[1];
    QString path = args[2];

    if(mode == "--check")
        checkFiles(path);
    else if(mode == "--create")
        createFile(path);
    else if(mode == "--createtree")
        createTree(path);
    else if(mode == "--checktree")
        checkTree(path);
    else
        return 0;
    return a.exec();
}

QByteArray fileChecksum(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly))
    {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f))
        {
            return hash.result();
        }
    }
    return QByteArray();
}

void checkFiles(QString path)
{
    QString fileName = path + "/checks.md5";
    QFile file(fileName);
    QList<QString> errors;
    QList<QString> notFound;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QStringList data;
        QTextStream textStream(&file);

        while (!textStream.atEnd())
        {
            data << textStream.readLine();
        }

        file.close();

        foreach (QString file, data)
        {
            QString path = file.split(':')[0] + ":" + file.split(':')[1];
            QString originMd5 = file.split(':')[2];
            QString md5 = fileChecksum(path, QCryptographicHash::Algorithm::Md5).toHex();
            QString string = path + ":" + md5;
            QFile f(path);
            if(!f.exists())
            {
                notFound.append(string);
                string.append(" NOT FOUND");
            }
            else
            {
                if(md5 == originMd5)
                {
                    string.append(" OK");
                }
                else
                {
                    string.append(" FAILED");
                    errors.append( path + ":" + md5);
                }
            }
            std::cout << string.toStdString() << "\n";
        }
        std::cout << "FINISHED" << "\n";

        std::cout << errors.count() << " failed" << "\n";
        if(errors.count() > 0)
        {
            foreach(QString error, errors)
                std::cout << error.toStdString() << "\n";
        }

        std::cout << notFound.count() << " not found" << "\n";

        if(notFound.count() > 0)
        {
            foreach(QString found, notFound)
                std::cout << found.toStdString() << "\n";
        }
    }
    else
    {
        std::cout << "FAILED" << "\n";
    }
}

void createFile(QString path)
{
    QDirIterator it(path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
    QList<QString> values;
    while (it.hasNext())
    {
        QString fullPath = it.next();
        QString md5 = fileChecksum(fullPath, QCryptographicHash::Algorithm::Md5).toHex();
        fullPath = fullPath.replace(path, "");
        QString string = fullPath + ":" + md5;
        std::cout << string.toStdString() << "\n";
        values.append(string);
    }
    std::cout << "FINISHED" << "\n";
    if(QFile::exists(path + "\\checks.md5"))
    {
        QFile::remove(path + "\\checks.md5");
    }
    else
    {
        QFile file(path + "\\checks.md5");
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            foreach(QString string, values)
            {
                stream << string << endl;
            }
        }
    }
}

void checkTree(QString path)
{
    QFile f(path + "\\checks.tree");
    if(!f.exists())
    {
        std::cout << "No checks.tree found" << "\n";
    }
    else
    {
        QDirIterator it(path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
        QList<QString> values;
        QList<QString> notFound;
        while (it.hasNext())
        {
            QString fullPath = it.next();
            std::cout << fullPath.toStdString() << "\n";
            values.append(fullPath);
        }

        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QStringList data;
            QTextStream textStream(&f);

            while (!textStream.atEnd())
            {
                data << textStream.readLine();
            }

            f.close();

            foreach(QString value, values)
            {
                QString info = value;
                if(data.contains(value))
                {
                    info.append(" OK");
                }
                else
                {
                    info.append(" NOT FOUND");
                    notFound.append(value);
                }
                std::cout << info.toStdString() << "\n";
            }

            std::cout << notFound.count() << " not found" << "\n";

            if(notFound.count() > 0)
            {
                foreach(QString found, notFound)
                    std::cout << found.toStdString() << "\n";
            }
        }
    }
}

void createTree(QString path)
{
    QDirIterator it(path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
    QList<QString> values;
    while (it.hasNext())
    {
        QString fullPath = it.next();
        QString string = fullPath;
        std::cout << string.toStdString() << "\n";
        values.append(string);
    }
    if(QFile::exists(path + "\\checks.tree"))
    {
        QFile::remove(path + "\\checks.tree");
    }
    else
    {
        QFile file(path + "\\checks.tree");
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            foreach(QString string, values)
            {
                stream << string << endl;
            }
        }
    }

    std::cout << "CREATED" << "\n";
}
