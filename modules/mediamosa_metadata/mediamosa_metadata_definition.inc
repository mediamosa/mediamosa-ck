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

/**
 * Definitions of CCK fields is in this function.
 *
 * @return array with CCK fields names as a key.
 *
 */
function _mediamosa_metadata_dc_qdc() {
  // Key is a metafield internal name, this is have to match name of MediaMosa
  // Value is an array with the following fields
  //   'label' - label using in Drupal interface;
  //   'read_only' - boolean, if TRUE Drupal can only display this data;
  //   'section' - can be 'group_dublincore', 'group_qualified_dublincore' or 'group_technical' use it to group data in section;
  //   'rdf' - property name for RDF, if differs from key value. For a reference:
  //      see http://www.w3.org/TR/xhtml-rdfa-primer/ , http://dublincore.org/documents/library-application-profile/index.shtml
  //

  static $fields = FALSE;

  if ($fields) {
    return $fields;
  }

  $dc_fields = array(
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

  $qdc_fields = array(
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

  // Some of the keys are named exactly as entries in metadata array inside Mediamosa.
  // Different story for 'mediafile_id', 'size', 'original'.
  //
  $tech_fields = array(
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
  foreach ($dc_fields as $name => &$item) {
    $item['section'] = 'group_dublincore';
    $item['readonly'] = FALSE;
  }

  // Set 'section' and 'readonly' to all QDC.
  foreach ($qdc_fields as $name => &$item) {
    $item['section'] = 'group_qualified_dublincore';
    $item['readonly'] = FALSE;
  }

  // Set 'section' and 'readonly' to all technical.
  foreach ($tech_fields as $name => &$item) {
    $item['section'] = 'group_technical';
    $item['readonly'] = TRUE;
    $item['rdf'] = FALSE;
  }

  $fields = $dc_fields;
  $fields += $qdc_fields;
  $fields += $tech_fields;

  // Set default RDF terms.
  foreach ($fields as $name => &$item) {
    if (!isset($item['rdf'])) {
      $item['rdf'] = $name;
    }
  }

  return $fields;
}

/**
 * Attaching our beautiful CCK fields to a content type.
 *
 * @param $content_type
 *   name of content type.
 *
 */
function _mediamosa_metadata_link_fields($content_type) {
  module_load_include('inc', 'content', 'includes/content.crud');

  $dc_fields = _mediamosa_metadata_dc_qdc();

  // Check for the first field presence.
  $somefield = array_shift(array_keys($dc_fields));
  $lookup_param = array('type_name' => 'mediamosa_videocontent', 'field_name' => 'field_meta_'. $somefield);
  $lookup = content_field_instance_read($lookup_param, TRUE);
  if (!empty($lookup)) {
    watchdog('Mediamosa CK', 'Mediamosa metadata fields are already exist. Fields has been NOT recreated, old data will be reused.', array(), WATCHDOG_INFO);
    return;
  }

  // Lets pre-fill common values.
  $field = array (
    'field_name' => 'TO BE FILLED',
    'type_name' => $content_type,
    'display_settings' =>
      array (
        'label' =>
          array (
            'format' => 'above',
            'exclude' => 0,
          ),
        'teaser' =>
          array (
            'format' => 'default',
            'exclude' => 0,
          ),
        'full' =>
          array (
            'format' => 'default',
            'exclude' => 0,
          ),
        4 =>
          array (
           'format' => 'default',
           'exclude' => 0,
          ),
        ),
    'widget_active' => '1',
    'type' => 'mediamosa_metadata',
    'required' => false,
    'multiple' => '0',
    'db_storage' => '1',
    'module' => 'mediamosa_metadata',
    'active' => '1',
    'locked' => '0',
    'columns' =>
      array (
        'value' =>
          array (
            'type' => 'varchar',
            'length' => 255,
            'not null' => false,
            'sortable' => true,
            'views' => true,
          ),
       ),
     'max_length' => '',
     'widget' =>
        array (
          'size' => NULL,
          'label' => 'TO BE FILLED',
          'weight' => '31',
          'description' => '',
          'type' => 'TO BE FILLED',
          'module' => 'mediamosa_metadata',
        ),
      );

  // Lets add our fields at the end of form.
  $weight = 40;

  foreach ($dc_fields as $field_name => $item) {
    $field['field_name'] = 'field_meta_'. $field_name;
    $field['widget']['label'] = $item['label'];
    $field['widget']['type'] = $item['readonly'] ? 'mediamosa_metadata_readonly' : 'mediamosa_metadata_readwrite';
    $field['widget']['weight'] = ++$weight;
    content_field_instance_create($field, FALSE);
  }

  $sections = _mediamosa_metadata_dc_qdc_fieldsets();
  foreach ($sections as $section_name => $section) {
    $section['type_name'] = $content_type;
    fieldgroup_save_group($content_type, $section);
  }

  // Seems that we have to put fields under group manually.
  foreach ($dc_fields as $field_name => $item) {
    db_query("INSERT INTO {%s} (type_name, group_name, field_name) VALUES ('%s', '%s', '%s')",
      fieldgroup_fields_tablename(), $content_type, $item['section'], 'field_meta_'. $field_name);
  }

  content_clear_type_cache(TRUE);
  menu_rebuild();
}

function _mediamosa_metadata_dc_qdc_fieldsets() {
  static $groups = FALSE;

  if ($groups) {
    return $groups;
  }

  $groups['group_dublincore'] = array(
    'group_type' => 'standard',
//    'type_name' => 'TO BE FILLED',
    'group_name' => 'group_dublincore',
    'label' => 'Dublin Core',
    'settings' => array(
      'form' => array(
        'style' => 'fieldset_collapsible',
        'description' => 'Dublin Core',
      ),
      'display' => array(
        'description' => 'Optional Dublin Core metadata descriptive fields',
        'teaser' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'full' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '2' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '3' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '4' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '33',
    'fields' => array(
      // '0' => 'TO BE FILLED',
      // '1' => 'TO BE FILLED',
    ),
  );

  $groups['group_qualified_dublincore'] = array(
    'group_type' => 'standard',
    //'type_name' => 'TO BE FILLED',
    'group_name' => 'group_qualified_dublincore',
    'label' => 'Qualified Dublin Core',
    'settings' => array(
      'form' => array(
        'style' => 'fieldset_collapsed',
        'description' => 'Qualified Dublin Core',
      ),
      'display' => array(
        'description' => 'Optional Qualified Dublin Core metadata descriptive fields',
        'teaser' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'full' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '2' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '3' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '4' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '34',
    'fields' => array(
      // '0' => 'TO BE FILLED',
      // '1' => 'TO BE FILLED',
    ),
  );

  $groups['group_technical'] = array(
    'group_type' => 'standard',
    //'type_name' => 'TO BE FILLED',
    'group_name' => 'group_technical',
    'label' => 'Technical metadata',
    'settings' => array(
      'form' => array(
        'style' => 'fieldset_collapsed',
        'description' => 'Technical metadata',
      ),
      'display' => array(
        'description' => 'Technical metadata descriptive fields',
        'teaser' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'full' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '2' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '3' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '4' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '35',
    'fields' => array(
      // '0' => 'TO BE FILLED',
      // '1' => 'TO BE FILLED',
    ),
  );

  $fields = _mediamosa_metadata_dc_qdc();

  foreach ($fields as $name => $info) {
    $groups[$info['section']]['fields'][] = $name;
  }

  return $groups;
}

/**
 * Destroying our fields and sections.
 *
 * @param $content_type
 *   name of content type.
 *
 */
function _mediamosa_metadata_unlink_fields($content_type) {
   module_load_include('inc', 'content', 'includes/content.crud');

  // Delete sections.
  $sections = _mediamosa_metadata_dc_qdc_fieldsets();
  foreach ($sections as $section) {
    fieldgroup_delete($content_type, $section['group_name']);
  }

  // Delete fields.
  $dc_fields = _mediamosa_metadata_dc_qdc();
  foreach ($dc_fields as $field_name => $field) {
    content_field_instance_delete('field_meta_'. $field_name, $content_type, FALSE);
  }
  // Clear caches and rebuild menu.
  content_clear_type_cache(TRUE);
  menu_rebuild();
}