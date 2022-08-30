#include "utils.h"

#include <utility>
#include <QMovie>
#include <QObject>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QThread>
#include <QTime>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

QString Utils_Uint8ToHexQStr(uint8_t in)
{
    return QString("%1").arg(in, 2, 16, QLatin1Char( '0' )).toUpper();
}

QString Utils_Uint8ArrToHexQStr(uint8_t *arr, ssize_t len)
{
    QString result = "";
    for( int i = 0; i < len; i++ )
    {
        result += Utils_Uint8ToHexQStr(arr[i]);
        if( i < len - 1 )
            result += " ";
    }
    return result;
}

QString Utils_QByteArrayToHexQStr(const QByteArray& bytes_arr)
{
    QString result = "";

    for( int i = 0; i < bytes_arr.count(); i++ )
    {
        result += Utils_Uint8ToHexQStr((uint8_t)bytes_arr.at(i)) + " ";
    }

    // Remove last space
    if( !result.isEmpty() )
        result.chop(1);

    return result;
}

bool Utils_RawHexStrToArr(QString in_hexstr, uint8_t *out_arr, ssize_t *out_len, ssize_t max_len)
{
    QByteArray bytes = Utils_RawHexStrToQByteArr(std::move(in_hexstr));
    *out_len = ( bytes.count() <= max_len ) ? bytes.count() : max_len;
    memcpy(out_arr, bytes.constData(), *out_len);
    return true;
}

QByteArray Utils_RawHexStrToQByteArr(QString in_hexstr)
{
    in_hexstr = in_hexstr.simplified().replace(",", "").replace("0x", "").replace("0X", "").replace(":", "").replace("\n", "");
    return QByteArray::fromHex(in_hexstr.toLatin1());
}

QString Utils_BytesToPrintableAsciiString(const QByteArray *in_arr)
{
    QString result = "";
    for(char i : *in_arr)
    {
        auto c = QChar(i);
        if( (c.unicode() >= 0x20 && c.unicode() <= 0x7f) || ( c.unicode() == 0x0A ) )
            result += c;
        else
            result += ".";
    }
    return result;
}

QString Utils_BytesToAlphanumericString(const QByteArray *in_arr)
{
    QString result = "";
    for(char i : *in_arr)
    {
        auto c = QChar(i);
        if( c.isDigit() || (c.unicode() >= 0x41 && c.unicode() <= 0x5A ) || (c.unicode() >= 0x61 && c.unicode() <= 0x7A ))
            result += c;
        else
            result += ".";
    }
    return result;
}

QString Utils_BytesToBinaryString(const QByteArray *in_arr)
{
    QString result = "";
    for(char i : *in_arr)
    {
        result += QString("%1").arg((uint8_t)i, 8, 2, QChar('0')) + " ";
    }
    return result.trimmed();
}

QString Utils_BytesToDECString(const QByteArray *in_arr)
{
    QString result = "";
    for(char i : *in_arr)
    {
        result += QString::number((uint8_t)i) + " ";
    }
    return result.trimmed();
}

QString ParseCertOrCsrFromFileToHexStr(const QString& fileName)
{
    // Open file and create reading stream
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly))
            return "";

    // Read file bytes
    QByteArray fileContent = f.readAll();

     // Read content into UI
    QString delimitator = "-----";
    if( QString(fileContent).contains(delimitator) )
    {
        // Convert certificate from B64 to binary format.
        QString input = QString(fileContent).simplified().replace(",", "").replace(" ", "").replace("\n", "");
        input = input.split(delimitator)[2].split(delimitator)[0];
        return QByteArray::fromBase64(input.toUtf8()).toHex(' ');
    }
    else
    {
        // Content already in as binary. Just send it to ui
        return fileContent.toHex(' ');
    }
}

QStringList Utils_ExtractAllUrls(const QString& inputText)
{   
    QStringList output;

    // https://www.geeksforgeeks.org/check-if-an-url-is-valid-or-not-using-regular-expression/
    QRegularExpression re("((http|https)://)[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)");

    re.setPatternOptions(QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);

    if( !re.isValid() )
    {
        qDebug().nospace().noquote()<< "Invalid matching regex for URLs extraction!";
        return output;
    }

    auto lines = inputText.split("\n");
    for( const QString &line: lines )
    {
        auto matches = re.globalMatch(line);
        while (matches.hasNext())
        {
            QRegularExpressionMatch match = matches.next();
            if (match.hasMatch())
            {
                QString currUrl = match.captured(0);

                // Remove case when domains like 'www.' are valid
                if(!currUrl.endsWith("www."))
                {
                    if( QUrl(currUrl, QUrl::ParsingMode::StrictMode).isValid() )
                    {
                        output.append(currUrl);
                    }
                }
            }
        }
    }

    // Remove duplicates
    output.removeDuplicates();

    return output;
}

QStringList Utils_ExtractAllHosts(const QString &input)
{
    QStringList output;

    output.append(input.split("\n"));

    output.removeAll(QString(""));

    return output;
}

bool Utils_IsValidIPv4(const QString &input)
{
    QHostAddress address(input);
    if (QAbstractSocket::IPv4Protocol == address.protocol())
    {
        return true;
    }
    return false;
}

bool Utils_IsValidIPv6(const QString &input)
{
    QHostAddress address(input);
    if (QAbstractSocket::IPv6Protocol == address.protocol())
    {
        return true;
    }
    return false;
}

void Utils_NumericListSort(QStringList *list)
{
    QMap<int, QString> m;
    for (const auto& s: *list) m[s.toInt()] = s;
    *list = QStringList(m.values());
}

bool Utils_FileExists(const QString& fileName)
{
    QFile file(fileName);
    return file.exists();
}

QStringList Utils_ParseCsvLine(const QString &string)
{
    enum State {Normal, Quote} state = Normal;
    QStringList fields;
    QString value = "";

    for (int i = 0; i < string.size(); i++)
    {
        const QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value.trimmed());
                value.clear();
            }

                // Double-quote
            else if (current == '"')
            {
                state = Quote;
                value += current;
            }

                // Other character
            else
                value += current;
        }

            // In-quote state
        else if (state == Quote)
        {
            // Another double-quote
            if (current == '"')
            {
                if (i < string.size())
                {
                    // A double double-quote?
                    if (i+1 < string.size() && string.at(i+1) == '"')
                    {
                        value += '"';

                        // Skip a second quote character in a row
                        i++;
                    }
                    else
                    {
                        state = Normal;
                        value += '"';
                    }
                }
            }

                // Other character
            else
                value += current;
        }
    }

    if (!value.isEmpty())
        fields.append(value.trimmed());

    // Quotes are left in until here; so when fields are trimmed, only whitespace outside of
    // quotes is removed.  The outermost quotes are removed here.
    for (auto & field : fields)
        if (field.length()>=1 && field.left(1)=='"')
        {
            field=field.mid(1);
            if (field.length()>=1 && field.right(1)=='"')
                field=field.left(field.length()-1);
        }

    // Treat case when string ends with comma. Just add an additional field
    if( string.endsWith(',') )
        fields.append("");

    return fields;
}

QString Util_EncodeForCSV(const QString &string)
{
    QString out = string;
    // Quotes shall be eascaped using double quotes
    if( out.contains("\"") )
        out = out.replace("\"", "\"\"");

    // If string contain commas, then content will be in double quotes
    if( out.contains(",") || out.contains("\n") )
        out = "\"" + out + "\"";

    return out;
}

QString Utils_FloatWithDigitsPrecision(float number, int precision)
{
    return QString::number(number , 'f', precision);
}

static QMovie *movieLoadingIcon = nullptr;
void Utils_PushButtonStartLoading(QPushButton *button)
{
    if( movieLoadingIcon == nullptr )
    {
        movieLoadingIcon = new QMovie();
        movieLoadingIcon->setFileName(":/img/loading.gif");
        movieLoadingIcon->start();
    }

    button->setEnabled(false);
    QObject::connect(movieLoadingIcon, &QMovie::frameChanged, button, [button]
    {
        button->setIcon(movieLoadingIcon->currentPixmap());
    });
}

void Utils_PushButtonEndLoading(QPushButton *button)
{
    button->setIcon(QIcon());
    button->setEnabled(true);

    QObject::disconnect(movieLoadingIcon, &QMovie::frameChanged, button, nullptr);
}

void Utils_Alert(const QString &title, const QString &message)
{
    QMessageBox msg;
    msg.setWindowTitle(title);
    msg.setText(message);
    msg.setIcon(QMessageBox::Warning);
    msg.exec();
}
void Utils_MsgBox(const QString &title, const QString &message)
{
    QMessageBox msg;
    msg.setWindowTitle(title);
    msg.setText(message);
    msg.setIcon(QMessageBox::Information);
    msg.exec();
}

void Utils_RichTextBoxPopup(const QString &title, const QString &content)
{
    auto *editor = new QPlainTextEdit(content);
    editor->setWindowTitle(title);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->setBaseSize(QSize(800, 400));
    editor->show();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
QString Utils_NestedQMapToString(const QVariantMap &map, int level)
{
    QString retStr;

    QString tabs = "";
    for( int i = 0; i < level; i++ )
        tabs += "\t";

    QMapIterator<QString, QVariant> it(map);
    while( it.hasNext() )
    {
        it.next();

        QString output = tabs + (level>0?"-->":"") + it.key();
        if( !it.value().toMap().isEmpty() )
        {
            retStr += output + "\n";
            retStr += Utils_NestedQMapToString(QVariantMap(it.value().toMap()), level + 1);
        }
        else
        {
            retStr += output + ": " + it.value().toString() + "\n";
        }
    }

    return retStr;
}
#pragma clang diagnostic pop

void SleepMs(quint64 ms)
{
    QTime dieTime = QTime::currentTime().addMSecs( ms );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
        QObject().thread()->usleep(1000*ms);
    }
}

#pragma clang diagnostic pop