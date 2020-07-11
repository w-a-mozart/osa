ALTER TABLE `civicrm_volunteer_need` 
ADD `description` TEXT CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL
  COMMENT 'Full description of the Volunteer Need. Text and HTML allowed. Displayed on sign-up screens.'
  AFTER `last_updated`;

