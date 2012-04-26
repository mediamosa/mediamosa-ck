<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 *  @file
 *  Dublin Core & Qualified Dublin Core fields definition goes here.
 */

assert(module_load_include('inc', 'field', 'field.crud') !== FALSE);

/**
 * Definitions of Metadata fields of MediaMosa.
 *
 * @return array
 *   MediaMosa metafield name as a key.
 *
 * Each element is an array with the following fields:
 *   - 'label'
 *     Label used in Drupal interface.
 *   - 'read_only'
 *     Boolean, if TRUE Drupal can only display this data;
 *   - 'section'
 *     Can be 'dublincore', 'qualified_dublincore' or 'technical' use it to
 *     group data in section;
 *   - 'rdf'
 *     Property name for RDF, if differs from key value.
 *      @see http://www.w3.org/TR/xhtml-rdfa-primer/ ,
 *           http://dublincore.org/documents/library-application-profile/index.shtml
 *   - 'field_name'
 *     Field name in Drupal.
 */
function _media_mediamosa_metadata_get_default() {

  static $fields = FALSE;

  if ($fields) {
    return $fields;
  }

  $fields_dc = array(
    'contributor' => array(
        'label' => t('Contributor'),
      ),
    'coverage_spatial' => array(
        'label' => t('Сoverage spatial'),
        'rdf'   => 'spatial',
      ),
    'coverage_temporal' => array(
        'label' => t('Сoverage temporal'),
        'rdf'   => 'temporal',
      ),
    'creator' => array(
        'label' => t('Creator'),
      ),
    'date' => array(
        'label' => t('Date'),
      ),
    'description' => array(
        'label' => t('Description'),
      ),
    'format' => array(
        'label' => t('Format'),
      ),
    'identifier' => array(
        'label' => t('Identifier'),
      ),
    'language' => array(
        'label' => t('Language'),
      ),
    'publisher' => array(
        'label' => t('Publisher'),
      ),
    'relation' => array(
        'label' => t('Relation'),
      ),
    'rights' => array(
        'label' => t('Rights'),
      ),
    'source' => array(
        'label' => t('Source'),
      ),
    'subject' => array(
        'label' => t('Subject'),
      ),
    'title' => array(
        'label' => t('Title'),
      ),
    'type' => array(
        'label' => t('Type'),
      ),
  );

  $fields_qdc = array(
    'created' => array(
        'label' => t('Created'),
      ),
    'description_abstract' => array(
        'label' => t('Description abstract'),
        'rdf'   => 'abstract',
      ),
    'format_extent' => array(
        'label' => t('Format extent'),
        'rdf'   => 'extent',
      ),
    'format_medium' => array(
        'label' => t('Format medium'),
        'rdf'   => 'medium',
      ),
    'hasformat' => array(
        'label' => t('Has format'),
        'rdf'   => 'hasFormat',
      ),
    'isformatof' => array(
        'label' => t('Is format of'),
        'rdf'   => 'isFormatOf',
      ),
    'isreferencedby' => array(
        'label' => t('Is referenced by'),
        'rdf'   => 'isReferencedBy',
      ),
    'issued' => array(
        'label' => t('Issued'),
      ),
    'license' => array(
        'label' => t('License'),
      ),
    'rightsholder' => array(
        'label' => t('Rightsholder'),
        'rdf'   => 'rights',
      ),
    'title_alternative' => array(
        'label' => t('Alternative title'),
        'rdf'   => 'alternative',
      ),
  );

  // Some of the keys are named exactly as entries in metadata array inside
  // Mediamosa.Different story for 'mediafile_id', 'size', 'original'.
  $fields_tech = array(
    'file_duration' => array(
        'label' => t('Duration'),
      ),
    'filesize' => array(
        'label' => t('File size'),
      ),
    'mediafile_id' => array(
        'label' => t('Mediafile ID'),
      ),
    'mime_type' => array(
        'label' => t('MIME type'),
      ),
    'container_type' => array(
        'label' => t('Container'),
      ),
    'size' => array(
        'label' => t('Video size'),
      ),
    'fps' => array(
        'label' => t('FPS'),
      ),
    'video_codec' => array(
        'label' => t('Video Codec'),
      ),
    'original' => array(
        'label' => t('Original'),
      ),
  );

  // Set 'section' and 'readonly' to all DC.
  foreach ($fields_dc as $name => &$field_dc) {
    $field_dc['section'] = 'dc';
    $field_dc['readonly'] = FALSE;
  }

  // Set 'section' and 'readonly' to all QDC.
  foreach ($fields_qdc as $name => &$field_qdc) {
    $field_qdc['section'] = 'qdc';
    $field_qdc['readonly'] = FALSE;
  }

  // Set 'section' and 'readonly' to all technical.
  foreach ($fields_tech as $name => &$field_tech) {
    $field_tech['section'] = 'mft';
    $field_tech['readonly'] = TRUE;
    $field_tech['rdf'] = FALSE;
  }

  // Now combine into one array.
  $fields = $fields_dc;
  $fields += $fields_qdc;
  $fields += $fields_tech;

  // Set default RDF terms.
  foreach ($fields as $name => &$field) {
    if (!isset($field['rdf'])) {
      $field['rdf'] = $name;
    }
  }

  // We're forced to introduce another element in addition to key because of 32
  // char limitation.
  foreach ($fields as $key => &$field) {
    $field['field_name'] = drupal_substr(FIELD_PREFIX . $key, 0, 32);
  }

  return $fields;
}

/**
 * Small helper for _media_mediamosa_metadata_get_default. Return array where
 * Drupal field name is a key and Mediamosa field name is a value, both plain
 * string.
 *
 * @see _media_mediamosa_metadata_get_default()
 */
function _media_mediamosa_metadata_get_default_mapped() {
  $fields = _media_mediamosa_metadata_get_default();

  foreach ($fields as $name => $item) {
    $mapping[$item['field_name']] = $name;
  }

  return $mapping;
}

/**
 * Attaching Metadata fields to all native Media entities.
 */
function _media_mediamosa_metadata_link_fields() {
  $fields_dc = _media_mediamosa_metadata_get_default();

  $media_types = media_type_get_types();

  $weight = 0;
  foreach ($fields_dc as $item) {
    $weight++;
    $field_name = $item['field_name'];
    $field = field_info_field($field_name);
    if (empty($field)) {
      $field = array(
        'field_name' => $field_name,
        'type' => 'text_long',
        'module' => 'media_mediamosa',
      );

      $field = field_create_field($field);
    }

    // Handy presets to use for 'default' array parameter.
    $display_show = array( 'label' => 'above', 'type' => 'text_default', 'module' => 'text', 'weight' => $weight);
    $display_off = array( 'type' => 'hidden');

    foreach ($media_types as $type_name => $media_type) {
      if (!$media_type->base) {
        continue;
      }
      $instance = field_info_instance('media', $field_name, $type_name);
      if (!empty($instance)) {
        watchdog('Mediamosa CK', 'No need to recreate instance for field @fieldname, it already exists in bundle @bundle.',
          array('@fieldname' => $field_name, '@bundle' => $type_name), WATCHDOG_INFO);
        continue;
      }

    $instance = array(
      'field_name' => $field_name,
      'entity_type' => 'media',
      'bundle' => $type_name,
      'label' => $item['label'],
      'required' => FALSE,

      // Widget goes here - responsible for editing.
      'widget_type' => 'text',
      'widget' => array(
        'weight' => $weight,
        'type' => 'text_textarea',
        'module' => 'text',
        'active' => TRUE,
      ),

      // Setup displaying - responsible for viewing.
      'display' => array(
        'default' => $display_show,
        'media_preview' => $display_show,
        'media_original' => $display_show,
        'media_link' => $display_off,
        'media_small' => $display_off,
        'media_large' => $display_show,
        ),
      );
      field_create_instance($instance);
    }
  }

  // Seems that in Drupal 7 we have to create two different section instances
  // for editing and displaying. So 'form' is a must, plus values for
  // displaying. See 'default' array above. Keeping with consistency is
  // recommended.
  $section_modes = array('form', 'default', 'media_preview', 'media_original', 'media_large');

  $format_settings_edit = array(
    'formatter' => 'closed',
    'instance_settings' => array(
      'classes' => '',
      'required_fields' => 1,
    ),
  );

  $format_settings_display = array(
    'formatter' => 'collapsed',
    'instance_settings' => array(
      'classes' => '',
      'description' => '',
    ),
  );

  $sections = _media_mediamosa_metadata_get_default_fieldsets();

  foreach ($sections as $section_name => $section) {
    foreach ($media_types as $type_name => $media_type) {
      foreach ($section_modes as $mode) {
        $section_instance = (object) $section;
        $section_instance->bundle = $type_name;
        $section_instance->mode = $mode;
        $section_instance->format_type = $section_instance->mode == 'form' ? 'tab' : 'fieldset';
        $section_instance->format_settings = $section_instance->mode == 'form' ? $format_settings_edit : $format_settings_display;
        // Create unique name.
        $group_name = $section_instance->group_name;
        $section_instance->identifier = $group_name . '|' . $type_name . '|' . $section_instance->entity_type . '|' . $mode;
        $section_instance->group_name = $section_instance->identifier;
        if (field_group_exists($section_name, 'media', $type_name, 'default')) {
          watchdog('Mediamosa CK', 'No need to recreate fieldgroup for @fieldgroup, it already exists in bundle @bundle.',
            array('@fieldgroup' => $section_name, '@bundle' => $type_name), WATCHDOG_INFO);
          continue;
        }
        field_group_group_save($section_instance);
      }
    }
  }

  cache_clear_all();
  menu_rebuild();
}


/**
 * Definitions of common part of fieldsets.
 *
 * @return array
 */
function _media_mediamosa_metadata_get_default_fieldsets() {
  static $groups = FALSE;

  if ($groups) {
    return $groups;
  }

  $group_core =
    array(
      'group_name' => 'TO BE FILLED',
      'bundle' => 'TO BE FILLED ON INSTANCE CREATION',
      'entity_type' => 'media',

      // Seems that in Drupal 7 we have to create two different section instances for editing and displaying.
      'mode' => 'TO BE FILLED ON INSTANCE CREATION',

      'parent_name' => '',
      'label' => 'TO BE FILLED',
      'weight' => 'TO BE FILLED',
      'children' => array(), // Fields will be stored here.
      'format_type' => 'TO BE FILLED ON INSTANCE CREATION: tab or fieldset',
      'format_settings' => 'TO BE FILLED ON INSTANCE CREATION',
      );

  // Initial weight for our sections.
  $weight = -8;

  // Names and labels array.
  $section_names = array(
    'dc' => 'Dublin Core Metadata',
    'qdc' => 'Qualified Dublin Core Metadata',
    'mft' => 'Mediafile Technical Metadata',
  );
  foreach ($section_names as $section_name => $section_label) {
    $groups[$section_name] = $group_core;
    $groups[$section_name]['group_name'] = $section_name;
    $groups[$section_name]['label'] = $section_label;
    $groups[$section_name]['weight'] = $weight++;
  }

  $fields = _media_mediamosa_metadata_get_default();

  // Finally, lets populate sections with fields.
  foreach ($fields as $item) {
    $groups[$item['section']]['children'][] = $item['field_name'];
  }

  return $groups;
}

/**
 * Removing metadata fields objects.
 */
function _media_mediamosa_metadata_unlink_fields() {
  mediamosa_ck::watchdog(__FUNCTION__  . ' executed', array(), WATCHDOG_DEBUG);
  $fields = _media_mediamosa_metadata_get_default();
  foreach ($fields as $item) {
    field_delete_field($item['field_name']);
  }

  MediaMediaMosa::watchdog('_media_mediamosa_metadata_unlink_fields done!', array(), WATCHDOG_DEBUG);
}
