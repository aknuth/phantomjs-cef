// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "print_handler.h"

#include "debug.h"

#include <QRect>

#include <algorithm>

QPrinter::PaperSize paperSizeForName(const QString& name)
{
  // NOTE: Must keep in sync with QPrinter::PaperSize
  static const QString sizeNames[] = {
    // Existing Qt sizes
    QStringLiteral("A4"),
    QStringLiteral("B5"),
    QStringLiteral("Letter"),
    QStringLiteral("Legal"),
    QStringLiteral("Executive"),
    QStringLiteral("A0"),
    QStringLiteral("A1"),
    QStringLiteral("A2"),
    QStringLiteral("A3"),
    QStringLiteral("A5"),
    QStringLiteral("A6"),
    QStringLiteral("A7"),
    QStringLiteral("A8"),
    QStringLiteral("A9"),
    QStringLiteral("B0"),
    QStringLiteral("B1"),
    QStringLiteral("B10"),
    QStringLiteral("B2"),
    QStringLiteral("B3"),
    QStringLiteral("B4"),
    QStringLiteral("B6"),
    QStringLiteral("B7"),
    QStringLiteral("B8"),
    QStringLiteral("B9"),
    QStringLiteral("C5E"),
    QStringLiteral("Comm10E"),
    QStringLiteral("DLE"),
    QStringLiteral("Folio"),
    QStringLiteral("Ledger"),
    QStringLiteral("Tabloid"),
    QStringLiteral("Custom"),

    // New values derived from PPD standard
    QStringLiteral("A10"),
    QStringLiteral("A3Extra"),
    QStringLiteral("A4Extra"),
    QStringLiteral("A4Plus"),
    QStringLiteral("A4Small"),
    QStringLiteral("A5Extra"),
    QStringLiteral("B5Extra"),

    QStringLiteral("JisB0"),
    QStringLiteral("JisB1"),
    QStringLiteral("JisB2"),
    QStringLiteral("JisB3"),
    QStringLiteral("JisB4"),
    QStringLiteral("JisB5"),
    QStringLiteral("JisB6"),
    QStringLiteral("JisB7"),
    QStringLiteral("JisB8"),
    QStringLiteral("JisB9"),
    QStringLiteral("JisB10"),

    // AnsiA = QStringLiteral("Letter"),
    // AnsiB = QStringLiteral("Ledger"),
    QStringLiteral("AnsiC"),
    QStringLiteral("AnsiD"),
    QStringLiteral("AnsiE"),
    QStringLiteral("LegalExtra"),
    QStringLiteral("LetterExtra"),
    QStringLiteral("LetterPlus"),
    QStringLiteral("LetterSmall"),
    QStringLiteral("TabloidExtra"),

    QStringLiteral("ArchA"),
    QStringLiteral("ArchB"),
    QStringLiteral("ArchC"),
    QStringLiteral("ArchD"),
    QStringLiteral("ArchE"),

    QStringLiteral("Imperial7x9"),
    QStringLiteral("Imperial8x10"),
    QStringLiteral("Imperial9x11"),
    QStringLiteral("Imperial9x12"),
    QStringLiteral("Imperial10x11"),
    QStringLiteral("Imperial10x13"),
    QStringLiteral("Imperial10x14"),
    QStringLiteral("Imperial12x11"),
    QStringLiteral("Imperial15x11"),

    QStringLiteral("ExecutiveStandard"),
    QStringLiteral("Note"),
    QStringLiteral("Quarto"),
    QStringLiteral("Statement"),
    QStringLiteral("SuperA"),
    QStringLiteral("SuperB"),
    QStringLiteral("Postcard"),
    QStringLiteral("DoublePostcard"),
    QStringLiteral("Prc16K"),
    QStringLiteral("Prc32K"),
    QStringLiteral("Prc32KBig"),

    QStringLiteral("FanFoldUS"),
    QStringLiteral("FanFoldGerman"),
    QStringLiteral("FanFoldGermanLegal"),

    QStringLiteral("EnvelopeB4"),
    QStringLiteral("EnvelopeB5"),
    QStringLiteral("EnvelopeB6"),
    QStringLiteral("EnvelopeC0"),
    QStringLiteral("EnvelopeC1"),
    QStringLiteral("EnvelopeC2"),
    QStringLiteral("EnvelopeC3"),
    QStringLiteral("EnvelopeC4"),
    // EnvelopeC5 = QStringLiteral("C5E"),
    QStringLiteral("EnvelopeC6"),
    QStringLiteral("EnvelopeC65"),
    QStringLiteral("EnvelopeC7"),
    // EnvelopeDL = QStringLiteral("DLE"),

    QStringLiteral("Envelope9"),
    // Envelope10 = QStringLiteral("Comm10E"),
    QStringLiteral("Envelope11"),
    QStringLiteral("Envelope12"),
    QStringLiteral("Envelope14"),
    QStringLiteral("EnvelopeMonarch"),
    QStringLiteral("EnvelopePersonal"),

    QStringLiteral("EnvelopeChou3"),
    QStringLiteral("EnvelopeChou4"),
    QStringLiteral("EnvelopeInvite"),
    QStringLiteral("EnvelopeItalian"),
    QStringLiteral("EnvelopeKaku2"),
    QStringLiteral("EnvelopeKaku3"),
    QStringLiteral("EnvelopePrc1"),
    QStringLiteral("EnvelopePrc2"),
    QStringLiteral("EnvelopePrc3"),
    QStringLiteral("EnvelopePrc4"),
    QStringLiteral("EnvelopePrc5"),
    QStringLiteral("EnvelopePrc6"),
    QStringLiteral("EnvelopePrc7"),
    QStringLiteral("EnvelopePrc8"),
    QStringLiteral("EnvelopePrc9"),
    QStringLiteral("EnvelopePrc10"),
    QStringLiteral("EnvelopeYou4"),
  };
  auto it = std::find_if(std::begin(sizeNames), std::end(sizeNames), [name] (const QString& size) {
    return !name.compare(size, Qt::CaseInsensitive);
  });

  if (it != std::end(sizeNames)) {
    auto distance = std::distance(std::begin(sizeNames), it);
    if (distance <= QPrinter::NPageSize) {
      return static_cast<QPrinter::PaperSize>(distance);
    }
  }
#if QT_VERSION >= 0x050300
  // Convenience overloads for naming consistency
  if (!name.compare(QLatin1String("AnsiA"), Qt::CaseInsensitive))
    return QPrinter::AnsiA;
  if (!name.compare(QLatin1String("AnsiB"), Qt::CaseInsensitive))
    return QPrinter::AnsiB;
  if (!name.compare(QLatin1String("EnvelopeC5"), Qt::CaseInsensitive))
    return QPrinter::EnvelopeC5;
  if (!name.compare(QLatin1String("EnvelopeDL"), Qt::CaseInsensitive))
    return QPrinter::EnvelopeDL;
  if (!name.compare(QLatin1String("Envelope10"), Qt::CaseInsensitive))
    return QPrinter::Envelope10;
#endif
  qCWarning(print) << "Unknown page size:" << name << "defaulting to A4.";
  return QPrinter::A4;
}

#define PHANTOMJS_PDF_DPI 72.0f

struct UnitConversion
{
    UnitConversion(const QLatin1String& unit, float factor)
        : unit(unit)
        , factor(factor)
    {}
    QLatin1String unit;
    float factor;
};

float stringToPointSize(const QString& string)
{
  static const UnitConversion units[] = {
    { QLatin1String("mm"), 72.0f / 25.4f },
    { QLatin1String("cm"), 72.0f / 2.54f },
    { QLatin1String("in"), 72.0f },
    { QLatin1String("px"), 72.0f / PHANTOMJS_PDF_DPI },
  };

  for (uint i = 0; i < sizeof(units) / sizeof(units[0]); ++i) {
    if (string.endsWith(units[i].unit)) {
      return string.midRef(0, string.size() - units[i].unit.size()).toFloat()
              * units[i].factor;
    }
  }
  return string.toFloat();
}

int stringToMillimeter(const QString& string)
{
  return round(stringToPointSize(string) / PHANTOMJS_PDF_DPI * 25.4f);
}

CefSize PrintHandler::GetPdfPaperSize(int device_units_per_inch)
{
  // this is just a default, we configure the size via CefPdfPrintSettings in handler.cpp
  QPrinter printer;
  printer.setResolution(device_units_per_inch);
  printer.setPaperSize(QPrinter::A4);
  auto rect = printer.paperSize(QPrinter::DevicePixel);
  return CefSize(rect.width(), rect.height());
}

bool PrintHandler::OnPrintDialog(bool has_selection, CefRefPtr<CefPrintDialogCallback> callback)
{
  qCDebug(print) << has_selection;
  return false;
}

bool PrintHandler::OnPrintJob(const CefString& document_name, const CefString& pdf_file_path, CefRefPtr<CefPrintJobCallback> callback)
{
  qCDebug(print) << document_name << pdf_file_path;
  return false;
}

void PrintHandler::OnPrintReset()
{
  qCDebug(print) << "!";
}

void PrintHandler::OnPrintSettings(CefRefPtr<CefPrintSettings> settings, bool get_defaults)
{
  qCDebug(print) << get_defaults;
}

#if CEF_COMMIT_NUMBER > 1333 // TODO: find correct commit number that adds this
void PrintHandler::OnPrintStart(CefRefPtr<CefBrowser> browser)
{
  qCDebug(print) << "!";
}
#endif
