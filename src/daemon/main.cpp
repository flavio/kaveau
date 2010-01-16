/* This file is part of the kaveau project
 *
 * Copyright (C) 2010 Flavio Castelli <flavio@castelli.name>
 *
 * kaveau is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * kaveau is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Keep; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kuniqueapplication.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include <signal.h>

#include "kaveaudaemon_export.h"
#include "kaveaudaemon.h"

namespace {
  static int installSignalHandler()
  {
    struct sigaction hup, term;

    hup.sa_handler = KaveauDaemon::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, 0) > 0)
      return 1;

    term.sa_handler = KaveauDaemon::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) > 0)
      return 2;

    return 0;
  }
}

namespace Kaveau {
  class DaemonApplication : public KUniqueApplication
  {
    public:
      DaemonApplication()
        : KUniqueApplication(false /* no gui */),
          m_daemon( 0 ) {
      }

      int newInstance() {
        if ( !m_daemon ) {
            m_daemon = new KaveauDaemon( this );
            //m_daemon->start();
        }
        return 0;
      }

    private:
      KaveauDaemon* m_daemon;
  };
}


extern "C" KAVEAU_DAEMON_EXPORT int kdemain ( int argc, char** argv )
{
  KAboutData aboutData( "KaveauDaemon", "kaveau",
                        ki18n("Kaveau Daemon"),
                        "0.1",
                        ki18n("Kaveau Daemon - Manages backups"),
                        KAboutData::License_GPL,
                        ki18n("(c) 2010, Flavio Castelli"),
                        KLocalizedString(),
                        "http://kaveau.kde.org" );
  aboutData.addAuthor(ki18n("Flavio Castelli"),ki18n("Maintainer"),
                      "flavio@castelli.name");

  KCmdLineArgs::init( argc, argv, &aboutData );

  KUniqueApplication::addCmdLineOptions();

  KComponentData componentData( &aboutData );

  if ( !KUniqueApplication::start() ) {
      fprintf( stderr, "Kaveau daemon already running.\n" );
      return 0;
  }

  installSignalHandler();

  Kaveau::DaemonApplication app;
  app.setQuitOnLastWindowClosed( false );
  return app.exec();
}
