/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

//! \file main.cpp
//! \brief Ofeli
//! \author Fabien Bessy
//! \version 1.1.0
//! \date April 2015

/*! \mainpage Developer's documentation
 *
 * \section intro_sec Introduction
 *
 * <p>Ofeli, as an acronym for <b>O</b>pen, <b>F</b>ast and <b>E</b>fficient <b>L</b>evel set
   <b>I</b>mplementation, demonstrates how to operate an image
   segmentation algorithm of Y. Shi and W. C. Karl <b>[1],</b> using a
   discrete approach for the approximation of level-set-based
   curve evolution (implicit active contours).</p>
   <p>This is a fast algorithm without the need
   of solving partial differential equations (PDE) while preserving the
   advantages of level set methods, such as the automatic handling of
   topological changes. Considerable speedups (×100) have been
   demonstrated as compared to PDE-based narrow band level-set implementations.</p>
   <p>The Home Page of Ofeli can be found at : https://sourceforge.net/projects/fastlevelset/ .</p>

 * \section structure_sec Structure
 *
 * In this project, the <b>G</b>raphical <b>U</b>ser <b>I</b>nterface (GUI) is clearly separated of the image processing part.
 *
 * GUI part :
 * - ImageWindow.cpp
 * - SettingsWindow.cpp
 * - AnalysisWindow.cpp
 * - CameraWindow.cpp
 * - CameraController.cpp
 * - AboutWindow.cpp
 * - LanguageWindow.cpp
 * - PixmapWidget.cpp
 * - ScrollAreaWidget.cpp
 *
 * Image processing part :
 * - List.tpp
 * - ActiveContour.cpp
 *      - RegionAc.cpp
 *      - RegionColorAc.cpp
 *      - EdgeAc.cpp
 * - HausdorffDistance.cpp
 * - Filters.cpp
 *
 *
 * \section reusability_sec Reusability
 *
 * <p>This Qt project is built as an application and not as a static or shared library. So if you are interested to use this C++ code, especially for the image processing part of the project,
 * you must just include this file(s) thanks to the preprocessor directive in your file(s). After you must just pass to each constructor an input argument pointer on a row-wise image data buffer and for the class ACwithoutEdgesYUV, a pointer on a RGB interleaved data buffer (R1 G1 B1 R2 G2 B2 ...).</p>
 * <p>If you prefer a command-line interface or if you are interested in a tracking example of this algorithm, you can find a fork of this project interfaced with Matlab (MEX-file). Each constructor takes an input pointer on a column-wise image data buffer and for the class ACwithoutEdgesYUV, a pointer on a RGB planar data buffer (R1 R2 R3 ... G1 G2 G3 ... B1 B2 B3 ...).</p>
 *
 *
 * \section license_sec License
 *
 * This software is distributed under the <a href='http://www.cecill.info/licences/Licence_CeCILL_V2-en.html'>CeCILL license version 2</a> <a href='http://www.cecill.info/licences/Licence_CeCILL_V2-fr.html'> (link to the french version here)</a>.
 *
 * \section acknowl_sec Acknowledgments
 *
 * Thanks to :
 * - J. Olivier, R. Boné, J-M. Girault, F. Amed, A. Lissy, C. Rouzière, L. Suta.
 * - <i>pattern recognition and image analysis research team</i>, <i>computer science laboratory</i>,
 * <i>François Rabelais University</i>.
 * - students and professors of the MSc in medical imaging.

<hr></hr>
* \section ref_sec References
<p><b>[1]</b> Y. Shi, W. C. Karl - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_MGIwYmUwYzctYTRkMC00ODMwLWI3YmUtNTFjYThlMTBkOTIy&hl=en&authkey=CPT1xeYN'>A real-time algorithm for the approximation of level-set based curve evolution</a> - <i>IEEE Trans. Image Processing</i>, vol. 17, no. 5, May 2008</p>
<p><b>[2]</b> T. F. Chan, L. A. Vese - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_NWY5ZGMyMmYtNzkwNi00NjI0LWE4ZGMtODllZTVmZWQ5NGRm&hl=en&authkey=CNfMkNEI'>Active contours without edges</a> - <i>IEEE Trans. Image Processing</i>, vol. 10, no. 2, Feb 2001</p>
<p><b>[3]</b> V. Caselles, R. Kimmel, G. Sapiro - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_ZWEzNzk2ZjgtNzlkMi00NDY0LTkzZjQtYWQ5N2EyNDA5NGE3&hl=en&authkey=CKi1w7cE'>Geodesic active contours</a> - <i>International Journal of Computer Vision</i>, 22(1), 61–79 (1997)</p>
<p><b>[4]</b> P. Perona, J. Malik - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_NmJmZWZkM2ItN2ZhZS00NjA4LTk3Y2UtNTNmYzkxYjFjNjU4&hl=en&authkey=CPDnxN8H'>Scale-space and edge detection using anistropic diffusion</a> - <i>IEEE Trans. Pattern Analysis and Machine Intelligence</i>, vol. 12, no. 17, Jul 1990</p>
 */

#include "application_settings.hpp"
#include "image_window.hpp"
#include "frame_clock.hpp"

#include <QTranslator>
#include <QLibraryInfo>
#include <QApplication>
#include <QCoreApplication>

int main( int argc, char* argv[] )
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("ofeli");
    QCoreApplication::setOrganizationDomain("https://sourceforge.net/projects/fastlevelset/");
    QCoreApplication::setApplicationName("ofeli");

    ofeli_app::FrameClock::init();

    const auto& config = ofeli_app::AppSettings::instance();
    ofeli_app::Language language = config.app_language;

    static QTranslator translator_qt;
    static QTranslator translator_ofeli;

    QString locale;
    switch(language)
    {
    case ofeli_app::Language::SYSTEM:
        locale = QLocale::system().name().section('_', 0, 0);
        break;
    case ofeli_app::Language::FRENCH:
        locale = "fr";
        break;
    case ofeli_app::Language::ENGLISH:
    default:
        locale = "en";
        break;
    }

    // Traductions Qt
    if (translator_qt.load("qt_" + locale,
                           QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    {
        app.installTranslator(&translator_qt);
    }

    // Traductions Ofeli
    if (translator_ofeli.load(QString(":/i18n/Ofeli_%1.qm").arg(locale)))
    {
        app.installTranslator(&translator_ofeli);
    }

    QIcon appIcon;
    appIcon.addFile(":/icons/Ofeli_16.png", QSize(16,16));
    appIcon.addFile(":/icons/Ofeli_32.png", QSize(32,32));
    appIcon.addFile(":/icons/Ofeli_48.png", QSize(48,48));
    appIcon.addFile(":/icons/Ofeli_128.png", QSize(128,128));
    appIcon.addFile(":/icons/Ofeli_256.png", QSize(256,256));

    QApplication::setWindowIcon(appIcon);
    qputenv("QT_QPA_PLATFORMTHEME", "kde");

    ofeli_app::ImageWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
