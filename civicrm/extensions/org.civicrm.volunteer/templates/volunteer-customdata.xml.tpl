<?xml version="1.0" encoding="iso-8859-1" ?>

<CustomData>
  <CustomGroups>
    <CustomGroup>
      <name>{$volunteer_custom_group_name}</name>
      <title>CiviVolunteer</title>
      <extends>Activity</extends>
      <extends_entity_column_value_option_group>activity_type</extends_entity_column_value_option_group>
      <extends_entity_column_value_option_value>{$volunteer_custom_activity_type_name}</extends_entity_column_value_option_value>
      <extends_entity_column_value>:;:;:;{$volunteer_activity_type_id}:;:;:;</extends_entity_column_value>
      <style>Inline</style>
      <collapse_display>1</collapse_display>
      <help_pre></help_pre>
      <help_post></help_post>
      <weight>2</weight>
      <is_active>1</is_active>
      <table_name>civicrm_value_civivolunteer_{$customIDs.civicrm_custom_group}</table_name>
      <is_multiple>0</is_multiple>
      <collapse_adv_display>0</collapse_adv_display>
      <is_reserved>1</is_reserved>
    </CustomGroup>
  </CustomGroups>
  <CustomFields>
    <CustomField>
      <name>Volunteer_Need_Id</name>
      <label>Volunteer Need</label>
      <data_type>Int</data_type>
      <html_type>Text</html_type>
      <is_required>0</is_required>
      <is_searchable>0</is_searchable>
      <is_search_range>0</is_search_range>
      <weight>1</weight>
      <is_active>1</is_active>
      <is_view>0</is_view>
      <text_length>255</text_length>
      <note_columns>60</note_columns>
      <note_rows>4</note_rows>
      <column_name>volunteer_need_id_{$customIDs.civicrm_custom_field}</column_name>
      <custom_group_name>CiviVolunteer</custom_group_name>
    </CustomField>
    <CustomField>
      <name>Volunteer_Role_Id</name>
      <label>Volunteer Role</label>
      <data_type>String</data_type>
      <html_type>Select</html_type>
      <is_required>0</is_required>
      <is_searchable>1</is_searchable>
      <is_search_range>0</is_search_range>
      <weight>2</weight>
      <is_active>1</is_active>
      <is_view>0</is_view>
      <text_length>64</text_length>
      <note_columns>60</note_columns>
      <note_rows>4</note_rows>
      <column_name>volunteer_role_id_{$customIDs.civicrm_custom_field+1}</column_name>
      <option_group_name>{$volunteer_custom_option_group_name}</option_group_name>
      <custom_group_name>CiviVolunteer</custom_group_name>
    </CustomField>
    <CustomField>
      <name>Time_Scheduled_Minutes</name>
      <label>Time Scheduled (minutes)</label>
      <data_type>Int</data_type>
      <html_type>Text</html_type>
      <is_required>0</is_required>
      <is_searchable>0</is_searchable>
      <is_search_range>0</is_search_range>
      <weight>2</weight>
      <is_active>1</is_active>
      <is_view>0</is_view>
      <text_length>255</text_length>
      <note_columns>60</note_columns>
      <note_rows>4</note_rows>
      <column_name>time_scheduled_in_minutes_{$customIDs.civicrm_custom_field+2}</column_name>
      <custom_group_name>CiviVolunteer</custom_group_name>
    </CustomField>
    <CustomField>
      <name>Time_Completed_Minutes</name>
      <label>Time Completed (minutes)</label>
      <data_type>Int</data_type>
      <html_type>Text</html_type>
      <is_required>0</is_required>
      <is_searchable>0</is_searchable>
      <is_search_range>0</is_search_range>
      <weight>3</weight>
      <is_active>1</is_active>
      <is_view>0</is_view>
      <text_length>255</text_length>
      <note_columns>60</note_columns>
      <note_rows>4</note_rows>
      <column_name>time_completed_in_minutes_{$customIDs.civicrm_custom_field+3}</column_name>
      <custom_group_name>CiviVolunteer</custom_group_name>
    </CustomField>
    <CustomField>
      <name>Credits</name>
      <label>Credits</label>
      <data_type>Number</data_type>
      <html_type>Text</html_type>
      <is_required>0</is_required>
      <is_searchable>0</is_searchable>
      <is_search_range>0</is_search_range>
      <weight>4</weight>
      <is_active>1</is_active>
      <is_view>0</is_view>
      <text_length>255</text_length>
      <note_columns>60</note_columns>
      <note_rows>4</note_rows>
      <column_name>credits_{$customIDs.civicrm_custom_field+3}</column_name>
      <custom_group_name>CiviVolunteer</custom_group_name>
    </CustomField>
  </CustomFields>
  <OptionGroups>
    <OptionGroup>
      <name>{$volunteer_custom_option_group_name}</name>
      <title>Volunteer Role</title>
      <is_reserved>1</is_reserved>
      <is_active>1</is_active>
    </OptionGroup>
  </OptionGroups>
  <OptionValues>
    <OptionValue>
      <label>Ticket-taker</label>
      <value>1</value>
      <name>Ticket_taker</name>
      <is_default>0</is_default>
      <weight>1</weight>
      <is_optgroup>0</is_optgroup>
      <is_reserved>0</is_reserved>
      <is_active>1</is_active>
      <option_group_name>volunteer_role</option_group_name>
    </OptionValue>
    <OptionValue>
      <label>Usher</label>
      <value>2</value>
      <name>Usher</name>
      <is_default>0</is_default>
      <weight>2</weight>
      <is_optgroup>0</is_optgroup>
      <is_reserved>0</is_reserved>
      <is_active>1</is_active>
      <option_group_name>volunteer_role</option_group_name>
    </OptionValue>
    <OptionValue>
      <label>Will Call</label>
      <value>3</value>
      <name>Will_Call</name>
      <is_default>0</is_default>
      <weight>3</weight>
      <is_optgroup>0</is_optgroup>
      <is_reserved>0</is_reserved>
      <is_active>1</is_active>
      <option_group_name>volunteer_role</option_group_name>
    </OptionValue>
  </OptionValues>
</CustomData>
