A repository to manage custom modules for **Drupal** and **CiviCRM**

This work is being done to support a non-profit children's music association.

All of this code is now in use.
One day, we'll get around to submitting to the appropriate project (this development is being done by a single volunteer in the wee hours of the night).

Some of the things included are:

1. **CiviCRM to Drupal commerce integration
- a CiviCRM custom payment processor which adds membership registration, event registration, other generic contributions to a Drupal commerce cart rather than an actual payment processor directly.

2. **Drupal to QuickBooks integration
- a Drupal module that uses the Drupal 7 Queue, the PHP DevKit from ConsoliBYTE and the Intuit QuickBooks Web Connector utility. This allows us to post a combination of Drupal Commerce orders and CiviCRM contribution records into a desktop version of QuickBooks.

~~paid CiviCRM relationships~~
3. ~~extend CiviCRM to enable end users to create and pay for a time limited relationship with another contact. Our use-case is to allow students to register and pay for private lesson fees with a specific teacher.~~
removed : students pay teacher directly of private lessons

4. **Drupal commerce payment processor for Elavon
- ~~Elavon (https://www.myvirtualmerchant.com/) payment processor for Drupal commerce using some of the code from the CiviCRM Elavon payment processor.~~
- redone, so that it uses the Converge (formally Elavon) Payment Form at https://classic.convergepay.com/virtualmerchant/ within an html frame to reduce PCI compliance requirements

5. **Search by Page for CiviCRM Events
- standard Drupal search doesn't search the CiviCRM Events descriptions. This module enables the full text indexing of CiviCRM Events by the Drupal Search by Page module.

6. **Very simple media gallery
- the himuesgallery module provides a simple images gallery module. We wanted to add a simple way to add links to youtube videos along with the photos and have them appear in a Colorbox. Note that this is a copy of the himuesgallery module (7.x-1.12) with our code. A patch has been submitted at http://Drupal.org/node/1595716. 

7. **CiviVolunteer modifications
- registration with the music school can be quite complicated so we have created a guided multi-page webform for a parent to register multiple children for membership and multiple group classes (civi events)
- volunteering is now a mandatory part of registration, so we have embedded the Angular CiviVolunteer opportunities page within a webform markup field

  Volunteer requirements are based on a "credit" system. Each **family** (civi household) must volunteer for activities totalling *X* credits, where X is determined by which events the children (household members) are registered as participants

- modified the volunteer need entity to include a **credit** field (and all of the html/js to support it)
- also added a **description** field to the volunteer need. With CiviVolunteer 4.7.31-2.3.1, the only way to differentiate volunteer needs for a given event, is through the volunteer role. Given our use case, we would have required 100+ roles, in order to provide enough detail to the volunteer.  This does not work well with the given implementation.
