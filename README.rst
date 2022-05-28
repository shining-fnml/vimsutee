=========
vimsutee
=========

-----------------------------------------------------
The silent evolution of Vim's "write with sudo" trick
-----------------------------------------------------

:Author: Lucio Tomarchio
:Date: 2022-05-27
:Copyright: GPL 3.0
:Version: 0.0.1
:Site: https://github.com/shining-fnml/kalacem

SYNOPSIS
========

  vimsutee [FILENAME]

DESCRIPTION
===========

Using **vim**'s `write with sudo <https://itecnote.com/tecnote/how-does-the-vim-write-with-sudo-trick-work/>`__ trick sometimes could be boring because user is always required to type the password.

**vimsutee** comes handy because it uses a different approach: if the user belongs to the same group as **vimsutee** installed binary his or her privileges are granted by the suid power.

**vimsutee** will refuse to overwrite anything if not launched from inside **vim** or its well known derivates such as **gvim**.


INSTALL
=======

Type ``make`` to build from source.

Before installation create the unix group *vimsutee*. If you want to name the group differently then edit 
the variable ``GROUP`` in *Makefile*.

Type ``sudo make install`` to install the binary.

**vimsutee** will be installed by default in */usr/bin*. If you want another path edit the variable ``PREFIX`` in *Makefile*. Note that **vimsutee** requires to be installed in a non suid-disabled filesystem to properly run.


EXAMPLES
========

Typical application
-------------------

Insert a line like the following in *.vimrc*:

::

    " Privileged write
    let drop=system("vimsutee")
    if v:shell_error == 0
        command W :execute ':silent w !vimsutee % > /dev/null' | :edit!
    else
        command W :execute ':w !sudo tee % > /dev/null' | :edit!
    endif

IF **vimsutee** (all of the following):

#. is found, readable and executable;
#. is installed as a group the user belongs to;
#. is not experiencing problems such an installation in a ``nosuid`` flagged filesystem;
#. detects **vim**'s binary [#]_ in environment PATH,

THEN **vim**'s variable ``shell_error`` is set to 0 and **vim**'s ``W`` command will run **vimsutee**.

ELSE the fallback is to use the traditional *sudo tee trick*.

SEE ALSO
========

* `sudo <man://sudo>`__
* `tee <man://tee>`__
* `vim <man://vim>`__

.. [#] Any of *gvim*, *vim*, *vim.gtk3* and *vim.tiny*
