#cuteNews

cuteNews is a user-friendly internet RSS news and podcast aggregator for Maemo5.

Features include:

    * Choice of 'OSSO' and 'Touch' user interfaces.
    * Choice of 'Dark on light' or 'Light on dark' reading modes.
    * Choice of URL, local file or custom command for subscription sources.
    * Search articles using keywords or SQLite WHERE query.
    * Option to save articles as favourites.
    * Option to automatically download enclosures.
    * Option to add custom URL openers for handling links and enclosures.


##Using custom commands for subscription source

The only requirement for a custom command is that it must output a valid RSS feed response to STDOUT, for example:

    wget -q http://talk.maemo.org/external?type=RSS2 -O -

##Custom URL openers

Custom URL openers can be managed either in the application, or by manually creating/editing the config file located at 
**~/.config/cuteNews/urlopeners**. Each URL opener has a display name, a regular expression (used to match the activated URL) 
and a command that will be executed when a URL is activated:

    [My MP3 handler]
    regExp=http(s|)://.+\\.mp3$
    command=/usr/bin/mp3player

Now, any link to an MP3 file will be opened using the specified command.
