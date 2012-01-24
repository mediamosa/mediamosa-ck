<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl)
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
 * @file
 * Theme related functions.
 */

/**
 * The theme used to generate a listing of mediafiles in the views using
 * mediamosa_ck_views_field_text_mediafiles.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_mediafiles($variables) {
  // Empty.
  $output = array();

  if (empty($variables['mediafiles'])) {
    return t('There are no other formats available');
  }

  // Loop.
  foreach ($variables['mediafiles'] as $mediafile) {
    if (empty($mediafile['metadata']['container_type'])) {
      continue; // No analysed yet.
    }
 
    // Build the link.
    $path = strtr($variables['asset_path'], array('[asset_id]' => $mediafile['asset_id'], '[mediafile_id]' => $mediafile['mediafile_id']));

    // Get filename.
    $output[] = '<li>';
  //  $output[] = t(drupal_strtoupper($mediafile['metadata']['container_type']));
  //  $output[] = ' ';
    $output[] = empty($mediafile['filename']) ? t('No filename present') : (l(trim($mediafile['filename']), $path));
    $output[] = '</li>';
  }

  return empty($output) ? '' : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * The theme used to generate a listing of collections in the views using
 * mediamosa_ck_views_field_text_collections.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_collections($variables) {
  // Empty.
  $output = array();

  if (empty($variables['collections'])) {
    return;
  }

  // Loop.
  foreach ($variables['collections'] as $collection) {
    $output[] = '<li>' . l($collection['title'], strtr($variables['collection_path'], array('[coll_id]' => $collection['coll_id']))) . '</li>';
  }

  return empty($output) ? '' : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * The theme used to generate a listing of collections in the views using
 * mediamosa_ck_views_field_text_metadata.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_metadata($variables) {
  // Empty.
  $rows = array();

  ksort($variables['metadata']);

  foreach ($variables['metadata'] as $name => $value) {
    $name = drupal_ucfirst(str_replace('_', ' ', $name));

    $rows[] = array('name' => t($name), 'value' => (empty($value) ? ' ' : nl2br(check_plain(t($value)))));
  }

  return theme('table', array('rows' => $rows));
}

/**
 * The theme used to generate a listing of collections in the views using
 * mediamosa_ck_views_field_text_technical_metadata.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_technical_metadata($variables) {
  // Empty.
  $rows = array();

  ksort($variables['technical_metadata']);

  foreach ($variables['technical_metadata'] as $name => $value) {
    $name = drupal_ucfirst(str_replace('_', ' ', $name));

    $rows[] = array('name' => t($name), 'value' => (empty($value) ? ' ' : t($value)));
  }

  return theme('table', array('rows' => $rows));
}

/**
 * The theme used to generate links to the edit/delete pages for collections.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_collection_actions($variables) {
  // Empty.
  $output = array();

  // @todo ; owner check.
  $output[] = '<li>' . l(t('edit'), strtr($variables['action_path_edit'], array('[coll_id]' => $variables['coll_id']))) . '</li>';
  $output[] = '<li>' . l(t('delete'), strtr($variables['action_path_delete'], array('[coll_id]' => $variables['coll_id']))) . '</li>';

  return empty($output) ? t('No actions') : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * The theme used to generate links to the edit/delete pages for assets.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_actions($variables) {
  // Empty.
  $output = array();

  // @todo ; owner check.
  $output[] = '<li>' . l(t('edit'), strtr($variables['action_path_edit'], array('[asset_id]' => $variables['asset_id']))) . '</li>';
  $output[] = '<li>' . l(t('delete'), strtr($variables['action_path_delete'], array('[asset_id]' => $variables['asset_id']))) . '</li>';

  return empty($output) ? t('No actions') : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * The theme will generate a link to the user using given data. When uid is 0 or
 * missing, then only the contents of user name is printed.
 *
 * @param type $variables
 */
function theme_mediamosa_ck_views_theme_owner($variables) {
  if (empty($variables['uid'])) {
    return empty($variables['name']) ? t('<no name>') : check_plain($variables['name']);
  }

  return l($variables['name'], 'user/' . $variables['uid']);
}
