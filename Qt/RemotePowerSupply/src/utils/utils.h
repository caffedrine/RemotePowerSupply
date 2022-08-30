#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QCryptographicHash>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>
#include <QUrl>
#include <QHostAddress>
#include <QPushButton>

QString Utils_Uint8ToHexQStr(uint8_t in);
QString Utils_Uint8ArrToHexQStr(uint8_t *arr, ssize_t len);
QString Utils_QByteArrayToHexQStr(const QByteArray& bytes_arr);
QByteArray Utils_RawHexStrToQByteArr(QString in_hexstr);
bool Utils_RawHexStrToArr(QString in_hexstr, uint8_t *out_arr, ssize_t *out_len, ssize_t max_len);

QString Utils_BytesToPrintableAsciiString(const QByteArray *in_arr);
QString Utils_BytesToAlphanumericString(const QByteArray *in_arr);
QString Utils_BytesToBinaryString(const QByteArray *in_arr);
QString Utils_BytesToDECString(const QByteArray *in_arr);

QString ParseCertOrCsrFromFileToHexStr(const QString& fileName);

QStringList Utils_ExtractAllUrls(const QString& inputText);
QStringList Utils_ExtractAllHosts(const QString &input);

bool Utils_IsValidIPv4(const QString &input);
bool Utils_IsValidIPv6(const QString &input);

void Utils_NumericListSort(QStringList *list);
bool Utils_FileExists(const QString& fileName);
QStringList Utils_ParseCsvLine(const QString &string);
QString Util_EncodeForCSV(const QString &string);
QString Utils_FloatWithDigitsPrecision(float number, int precision = 2);

template <class T>
static QString join(const QList<T> &list,
                    const QString &separator,
                    const std::function< QString (const typename QList<T>::value_type &) > toStringFunction);

void Utils_PushButtonStartLoading(QPushButton *button);
void Utils_PushButtonEndLoading(QPushButton *button);
void Utils_Alert(const QString &title, const QString &message);
void Utils_MsgBox(const QString &title, const QString &message);
void Utils_RichTextBoxPopup(const QString &title, const QString &content);

// Data structures utils
QString Utils_NestedQMapToString(const QVariantMap &map, int level = 0);

void SleepMs(quint64 ms);

#endif // UTILS_H

#pragma clang diagnostic pop