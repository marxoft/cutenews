/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef URLOPENER_H
#define URLOPENER_H

#include <QObject>
#include <QRegExp>

typedef struct {
    QString name;
    QRegExp regExp;
    QString command;
} Opener;

class UrlOpener : public QObject
{
    Q_OBJECT

public:
    ~UrlOpener();
    
    static UrlOpener* instance();
    
    QList<Opener> openers() const;
    Q_INVOKABLE QString opener(const QString &url) const;

    Q_INVOKABLE bool canOpen(const QString &url) const;

public Q_SLOTS:
    void load();
    
    bool addOpener(const QString &name, const QString &regExp, const QString &command);
    void removeOpener(const QString &name);
    
    bool open(const QString &url);

private:
    UrlOpener();
    
    static UrlOpener *self;
    
    QList<Opener> m_openers;
};

#endif // URLOPENER_H
