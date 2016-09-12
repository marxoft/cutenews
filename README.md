#cuteNews

cuteNews is a user-friendly internet RSS news and podcast aggregator for Maemo5 and Linux desktop that can be extended to support feeds from any source using plugins.

Features include:

    * Choice of 'OSSO' and 'Touch' user interfaces (Maemo5).
    * Choice of 'Dark on light' or 'Light on dark' reading modes (Maemo5).
    * Choice of URL, local file, custom command or plugin for subscription sources.
    * Import subscriptions via OPML.
    * Search articles using keywords or SQLite WHERE query.
    * Option to save articles as favourites.
    * Option to automatically update subscriptions.
    * Option to automatically delete read articles.
    * Option to automatically download enclosures.
    * Option to add custom URL openers for handling links and enclosures.


##Using custom commands for subscription source

The only requirement for a custom command is that it must output a valid RSS feed response to STDOUT, for example:

    wget -q http://talk.maemo.org/external?type=RSS2 -O -

##Custom URL openers

Custom URL openers can be managed either in the application, or by manually creating/editing the config file located at 
**~/.config/cutenews/urlopeners**. Each URL opener has a display name, a regular expression (used to match the activated URL) 
and a command that will be executed when a URL is activated:

    [My MP3 handler]
    regExp=http(s|)://.+\.mp3$
    command=/usr/bin/mp3player

Now, any link to an MP3 file will be opened using the specified command.
