
Welcome to "himuesgallery", a very simple to install and very simple to use picture-gallery for Drupal 6.
With himuesgallery creating a gallery is as simple as copy your picturefiles on your webspace and create a new node for the gallery.
But himuesgallery is very flexible to show your pictures using css-files (some will be delivered with this module, but you can create your own ones).

Installation
============
You install himuesgallery in three simple steps:

1.) unzip himuesgallery.tgz to your sites/all/modules/ - folder
2.) activate himues-gallery in http://<yoursite>/admin/build/modules. You'll find it in section "himues developement"
3.) go to http://<yoursite>/admin/himuesgallery and type in the path and folder under sites/default/files where your picture-dirs exist.

Thats all.

Quick createation of a gallery
==============================
himuesgallery creates a new content type. So you create a new gallery as simply as create a story.
- Copy your pictures in a subfolder of the base-folder you defined in "Installation->Step 3. You can put the files on webspace using any filemanager or your favorite ftp-program.
- Click on "create content" and then on "himuesgallery" or navigate to http://<yoursite>/node/add/himuesgallery
- Name your Gallerie (Title)
- Select a CSS-File from dropdown-box
- Select the directory with pictures from dropdown-box (remember: the gallery-dirs are subfolders from the dir you have defined in Installation->Step 3.)
- Select how to show the pictures if you click on the thumbnail (only makes sense if you install and enable Lightbox 2 on Drupal 6 or Colorbox on Drupal 7.
  Otherwise a click on a thumbnail open the original picture in a new window.)
- Select the sort-order in wich the pictures are shown (ascending, descending)
- If you wish, you can create a text, which will be shown in top of the gallery. If you have installed a wysiwyg-editor you can write wysiwyg-text.
- Optionaly select if thumbnail will be crated with correct aspect ratio
- Optionally give the gallery a menu-point
- Optionally make the gallery a page of a book
- save the gallery
Thats it!
You see the gallery and the filenames will be used as picture-description (without .jpg, .gif, .png). No need to name the pictures everywhere.

Other things you can do
=======================

Numbering files:
----------------
If you name your files like ###-Filename.jpg where ### is an 3-digit-integer, than the files will be sorted by the integer, but it will not be shown in the filedescription.

Example:
 - you have two files named "001-This is the first file.jpg" and "002-Another file.JPG"
 - the Filenames will be used as description of the tumbnail an will be shown as "This is the first file" and "Another file".
Integers must not be one after another. So you can name files like "001-File1.jpg", "005-File2.jpg", "010-File3.jpg".
In this way you can order your pictures in any way you want.

Use text-decoration in filenames/description:
---------------------------------------------
While you cant use html-code in filenames there are some alternate characters you can use.

Example:
 - You name your picture-file "001-This is .red.RED.-red. text.jpg". The description below the thumbnail looks like "This is RED text" where the word RED ist in red color.
 
Alternate text-decoration characters you can use:
	.br.         = line-break
	.i.          = italic on
	.-i.         = italic off
	.b.          = bold on
	.-b.         = bold off
	.ap.         = apostroph
	.es.         = apostroph (for german users: es = einfache Anfuehrungsstriche)
	.qm.         = quotation mark
	.af.         = quotation mark (for german users: af = Anfuehrungsstriche)
	.bs.         = backslash (\)
	.red.        = red text on
	.-red.       = red text off
	.blue.       = blue text on
	.-blue.      = blue text off
	.green.      = green text on
	.-green.     = green text off
	.qe.         = question mark
	.fz.         = question mark (for german users: fz = Fragezeichen)
	.right.      = right-align text on
	.-right.     = right-align text off
	.left.       = left-align text on
	.-left.      = left-align text off
	_            = underlines will be replaced with space
        ".center.",  = center text on
        ".-center.", = center text off
        ".auml",     = German Umlaut ä (only necessary if you have prolems using utf8)
        ".Auml.",    = German Umlaut Ä                       "
        ".ouml.",    = German Umlaut ö                       "
        ".Ouml.",    = German Umlaut Ö                       "
        ".uuml.",    = German Umlaut ü                       "
        ".Uuml.",    = German Umlaut Ü                       "
        ".szlig.",   = German Umlaut ß                       "
                                     

Use Links in picture-description
-----------------------------
As you can't put html-links into filenames, there is another way to use links in picture-description:
  You have to create a text-file that has the same name as the picture but ".inc" as extension. In description you put the tag ".link." where the link should be shown.
  ".inc" has following format:
    line 1: Text that should be shown in description
    line 2: link-adress
    line 3: link destination (_blank, _self, _parent, _top)
                         
  e.g. filename of picture  is 001-Link .link. to somewhere.jpg
  filename of linkfile is 001-Link .link. to somewhere.inc
                                  
  .inc-file has following content:
     me
     http://drupal.org
     _blank
                                                              
  Result is the Description: Link me to somewhere
  where "me" is a link to http://drupal.org
                                                                            
                                                                            

Use Lightbox 2 / Colorbox
-------------------------
If you installed and activated Lightbox 2 (Drupal 6) or Colorbox (Drupal 7), you can chose what happen, if you click on an gallery-thumbnail.
 - you can show that picture as single picture in a lightbox
 - you can show that picture as single picture in a lightbox whith arrows to show last/next picture in lightbox
 - you can show all the pictures in an automatic lightshow
 
Use your own css-files
----------------------
You can put your own css-files in the subfolder "css" in the module-dir or you can put a css-file named himuesgallery.css in the picture-dir and select the entry "--- himuesgallery.css in PictureDir ---" from css-dropdownbox during gallery-creation to use it.
Use the existing css-files in module-dir as example.


Tips
====

Naming the gallery-dirs
-----------------------
I name my gallery-dirs like "2010-one dir", "2009-another-dir". You can also name it ""2010-01-30-one dir", "2009-12-24-another-dir".
In the dropdownbox in gallery-creation the dirs will be shown in descending order, so the newest folder will be on top.

Changing files in picture-folder
--------------------------------
If you change/delete/add files in picture-folder you should go to edit the gallery and save it without any changes.
In this case the thumbnails will be renewed.


Examples on live-site
=====================
If you whish to see himuesgallery live got to http://st-michael.de/gemeindeleben
Here you find several menu-entrys (2010, 2009, 2008). this entrys are books. Click on one (eg. 2009). The book-pages all are gallerys. Maybe click on "Sternsinger" to see one.
Click on the thumbnails to see what happens.

Another example on the same site is our pressarchiv under http://st-michael.de/pressearchiv

Last but not least
==================
- If you like himuesgallery and use it on your own site, I'm glad to get a mail from you to himuesgallery@himue.com
  Perhaps you can tell me the url of the side?
  
- Englisch is'nt my first language, so if there are any misspelling in the program or in this readme, please tell me.

- If you find any error or if you have any suggestion for improvement you are welcome.

- If you like to change himuesgallery I'll be glad to get a copy of the new version.

- My first language is german, so you can write in german or in english.

Greetings from germany,
Himue