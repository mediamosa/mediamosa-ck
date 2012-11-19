<?php
/**
 * @file
 * Default views integration for the Mediamosa Media Browser module.
 *
 * @copyright
 * Copyright (c) 2012 Palantir.net.
 */

/**
 * Implements hook_views_default_views().
 */
function media_mediamosa_views_default_views() {

  $view = new view();
  $view->name = 'mediamosa_asset_search';
  $view->description = 'Show a listing assets for searching results.';
  $view->tag = 'asset, listing, search';
  $view->base_table = 'mediamosa_asset_search';
  $view->human_name = 'MediaMosa Browser';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'MediaMosa Browser';
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['submit_button'] = 'Search';
  $handler->display->display_options['exposed_form']['options']['autosubmit_hide'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '20';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['quantity'] = '9';
  $handler->display->display_options['style_plugin'] = 'media_mediamosa_browser';
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['label'] = 'Your search yielded no results';
  $handler->display->display_options['empty']['area']['content'] = '<h2>Your search yielded no results</h2>
  Check if your spelling is correct.
  Remove quotes around phrases to search for each word individually. bike shed will often show more results than "bike shed".
  Consider loosening your query with OR. bike OR shed will often show more results than bike shed.
  ';
  $handler->display->display_options['empty']['area']['format'] = 'full_html';
  /* Field: MediaMosa: dc.title */
  $handler->display->display_options['fields']['dc.title']['id'] = 'dc.title';
  $handler->display->display_options['fields']['dc.title']['table'] = 'mediamosa_asset_search';
  $handler->display->display_options['fields']['dc.title']['field'] = 'dc.title';
  /* Field: MediaMosa: Mediafile duration */
  $handler->display->display_options['fields']['mediafile_duration']['id'] = 'mediafile_duration';
  $handler->display->display_options['fields']['mediafile_duration']['table'] = 'mediamosa_asset_search';
  $handler->display->display_options['fields']['mediafile_duration']['field'] = 'mediafile_duration';
  $handler->display->display_options['fields']['mediafile_duration']['label'] = '';
  $handler->display->display_options['fields']['mediafile_duration']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['mediafile_duration']['date_format'] = 'mediamosa_ck_mf_duration';
  /* Field: MediaMosa: Created */
  $handler->display->display_options['fields']['videotimestamp_1']['id'] = 'videotimestamp_1';
  $handler->display->display_options['fields']['videotimestamp_1']['table'] = 'mediamosa_asset_search';
  $handler->display->display_options['fields']['videotimestamp_1']['field'] = 'videotimestamp';
  $handler->display->display_options['fields']['videotimestamp_1']['label'] = '';
  $handler->display->display_options['fields']['videotimestamp_1']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['videotimestamp_1']['date_format'] = 'mediamosa_ck_normal';
  /* Sort criterion: MediaMosa: Created */
  $handler->display->display_options['sorts']['videotimestamp']['id'] = 'videotimestamp';
  $handler->display->display_options['sorts']['videotimestamp']['table'] = 'mediamosa_asset_search';
  $handler->display->display_options['sorts']['videotimestamp']['field'] = 'videotimestamp';
  $handler->display->display_options['sorts']['videotimestamp']['order'] = 'DESC';
  /* Filter criterion: MediaMosa: Search metadata */
  $handler->display->display_options['filters']['metadata']['id'] = 'metadata';
  $handler->display->display_options['filters']['metadata']['table'] = 'mediamosa_asset_search';
  $handler->display->display_options['filters']['metadata']['field'] = 'metadata';
  $handler->display->display_options['filters']['metadata']['value'] = array(
    'metadata' => '',
  );
  $handler->display->display_options['filters']['metadata']['exposed'] = TRUE;
  $handler->display->display_options['filters']['metadata']['expose']['operator_id'] = '';
  $handler->display->display_options['filters']['metadata']['expose']['label'] = 'Search metadata';
  $handler->display->display_options['filters']['metadata']['expose']['operator'] = 'metadata_op';
  $handler->display->display_options['filters']['metadata']['expose']['identifier'] = 'metadata';
  $handler->display->display_options['filters']['metadata']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );

  $views[$view->name] = $view;

  return $views;
}
