/*****************************************************************************
 * mx-codecs.cpp
 *****************************************************************************
 * Copyright (C) 2014 MX Authors with the exeption of getCmdOut function
 *  getCmdOut function copyright (C) 2003-2014 Warren Woodford 
 *   released under the Apache License version 2.0
 * 
 * Authors: Jerry 3904
 *          Anticaptilista
 *          Adrian
 *          MEPIS Community <http://forum.mepiscommunity.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "mxcodecs.h"
#include "ui_mxcodecs.h"

#include <stdio.h>

#include <QDesktopServices>
#include <QUrl>
#include <QDir>


mxcodecs::mxcodecs(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::mxcodecs)
{
  ui->setupUi(this);
}

mxcodecs::~mxcodecs()
{
  delete ui;
}

void mxcodecs::updateStatus(QString msg, int val) {
  ui->labelDownload->setText(msg.toAscii());
  ui->progressBar->setValue(val);
  qApp->processEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Util function taken from minstall, part of MEPIS, Copyright (C) 2003-2010 by Warren Woodford
// Licensed under the Apache License, Version 2.0

QString mxcodecs::getCmdOut(QString cmd) {
  char line[260];
  const char* ret = "";
  FILE* fp = popen(cmd.toAscii(), "r");
  if (fp == NULL) {
    return QString (ret);
  }
  int i;
  if (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    ret = line;
  }
  pclose(fp);
  return QString (ret);
}


/////////////////////////////////////////////////////////////////////////
// general

void mxcodecs::on_buttonOk_clicked() {
  if (ui->stackedWidget->currentIndex() == 0) {
    setCursor(QCursor(Qt::WaitCursor));
    installDebs(downloadDebs());
    setCursor(QCursor(Qt::ArrowCursor));
  } else {
    qApp->exit(0);
  }
}

//download .deb codecs returns download path
QString mxcodecs::downloadDebs() {
  QString cmd, out, msg;
  QString path, arch;
  QString url = "http://deb-multimedia.org";

  //set progressBar and refresh
  ui->progressBar->setValue(0);
  ui->stackedWidget->setCurrentIndex(1);
  qApp->processEvents();

  // create temp folder and set it current
  path = getCmdOut("mktemp -d");
  QDir dir(path);
  dir.mkdir(path);
  dir.setCurrent(path);

  // get arch info
  arch = getCmdOut("dpkg --print-architecture");

  cmd = "wget -qO- " + url + "/dists/stable/main/binary-" + arch + "/Packages.gz | zgrep ^Filename | grep libdvdcss2 | awk \'{print $2}\'";
  updateStatus(tr("<b>Running command...</b><p>") + cmd, 10);
  out = getCmdOut(cmd);
  if (out == "") {
    QMessageBox::critical(0, tr("Error"),
                          tr("Cannot connect to the download site"));
  } else {
    cmd = "wget -q " + url + "/" + out;
    updateStatus(tr("<b>Running command...</b><p>") + cmd, 20);
    if (system(cmd.toAscii()) != 0) {
      QMessageBox::critical(0, QString::null,
                            QString(tr("Error downloading %1")).arg(out));
    }
  }

  cmd = "wget -qO- " + url + "/dists/stable/non-free/binary-" + arch + "/Packages.gz | zgrep ^Filename | grep w.*codecs | awk \'{print $2}\'";
  updateStatus(tr("<b>Running command...</b><p>") + cmd, 50);
  out = getCmdOut(cmd);
  if (out == "") {
    QMessageBox::critical(0, tr("Error"),
                          tr("Cannot connect to the download site"));
  } else {
    cmd = "wget -q " + url + "/" + out;
    updateStatus(tr("<b>Running command...</b><p>") + cmd, 70);
    if (system(cmd.toAscii()) != 0) {
      QMessageBox::critical(0, tr("Error"),
                            QString(tr("Error downloading %1")).arg(out));
    }
  }

  updateStatus(tr("<b>Download Finished.</b>"), 100);

  return path;
}

//install downloaded .debs
void mxcodecs::installDebs(QString path) {
  QString cmd, out, msg;
  QDir dir(path);
  dir.setCurrent(path);
  bool error = false;

  //filter *.deb file only
  QStringList filter;
  filter << "*.deb";
  dir.setNameFilters(filter);

  QStringList fileList = dir.entryList();

  ui->groupBox->setTitle(tr("Installing downloaded files"));

  int size = fileList.size();
  if (size == 0) {
    QMessageBox::critical(0, tr("Error"),
                             tr("No downloaded *.debs files found."));
    qApp->exit(1);
  }

  while (!fileList.isEmpty()) {
    QString file = fileList.takeFirst();
    cmd = QString("dpkg -i %1").arg(file);
    updateStatus(tr("<b>Installing...</b><p>")+file, 100/(fileList.size()+1)-100/size);
    if (system(cmd.toAscii()) != 0) {
      QMessageBox::critical(0, QString::null,
                            QString(tr("Error installing %1")).arg(file));
      error = true;
    }
    dir.remove(file);
  }

  dir.rmdir(path);
  ui->groupBox->setTitle("");
  updateStatus(tr("<b>Installation process has finished</b>"), 100);

  if (!error) {
    QMessageBox::information(0, tr("Finished"),
                             tr("Codecs files have been downloaded and installed successfully."));
    qApp->exit(0);
  } else {
    QMessageBox::critical(0, tr("Error"),
                             tr("Process finished. Errors have occurred during the installation."));
    qApp->exit(1);
  }
}


/////////////////////////////////////////////////////////////////////////
// slots


// show about
void mxcodecs::on_buttonAbout_clicked() {
  QMessageBox msgBox(QMessageBox::NoIcon, tr("About MX Codecs Installer"),
                     tr("<p align=\"center\"><b><h2>MX Codecs Installer</h2></b></p><p align=\"center\">MX14+git20140221</p><p><h3>Simple codecs\
      downloader for antiX MX</h3></p><p align=\"center\"><a href=\"http://www.mepiscommunity.org/mx\">\
      http://www.mepiscommunity.org/mx</a><br /></p><p align=\"center\">Copyright (c) antiX<br /><br /></p>"), 0, this);
      msgBox.addButton(tr("License"), QMessageBox::AcceptRole);
                     msgBox.addButton(QMessageBox::Cancel);
      if (msgBox.exec() == QMessageBox::AcceptRole)
      QDesktopServices::openUrl(QUrl("file:///usr/local/share/doc/mx-codecs-license.html"));
}

void mxcodecs::on_buttonHelp_clicked() {
  QDesktopServices::openUrl(QUrl("file:///usr/local/share/doc/mxapps.html"));
}
