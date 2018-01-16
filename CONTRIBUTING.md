# QDSP Contribution Guide

Want to help improve QDSP? Just fork the code on GitHub and make a pull request.

## Git workflow

QDSP uses [Vincent Driessen's git flow
model](http://nvie.com/posts/a-successful-git-branching-model/). All changes
should be made on the `develop` branch, or made on feature branches and merged
into `develop`.

## Style

Most of this code base uses "smart tabs": tabs for indentation and spaces for
alignment. That way, everyone can set the tab width to whatever they like (I
personally prefer 6) and everything will still line up. This isn't a hard and
fast rule; I'll still accept your pull request no matter what style you use, but
I highly recommend sticking with smart tabs for consistency.

You can automate this in both [Emacs](https://www.emacswiki.org/emacs/SmartTabs)
and [Vim](https://github.com/vim-scripts/Smart-Tabs).
