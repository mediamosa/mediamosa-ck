<?php
/**
 * @file
 * Admin page callbacks for the MediaMosa CK media management module.
 */

/**
 * Show the media management page.
 */
function _mediamosa_ck_media_management_page() {
  drupal_set_title(t('Mediamanagement'));
  $page = array();
  $page[] = '<div id="mediamosa-ck-cs-popup-left"></div>';
  $page[] = '<div id="mediamosa-ck-cs-popup-full"></div>';
  $page[] = '<div id="mediamosa-ck-cs-popup-right"></div>';

  // Add the search form.
  $variables['left'] = _mediamosa_ck_media_management_search_panel();

  // Upload contents.
  $variables['right'] = _mediamosa_ck_media_management_batch_panel();

  $page[] = theme('mediamosa_ck_two_column_page', $variables);

  return implode('', $page);
}

// ------------------------------------------------------- Search panel + forms.
/**
 * Left search panel.
 *
 * @return string
 *   The search panel in HTML.
 */
function _mediamosa_ck_media_management_search_panel() {

  // Get the asset selection form.
  $search_form = drupal_get_form('mediamosa_ck_media_management_search_form');
  $media_management_search_form = drupal_render($search_form);

  $variables = array(
    'tabs' => array(
      array(
        'title' => 'Search',
        'content' => $media_management_search_form,
      ),
      array(
        'title' => 'Search result',
        'content' => _mediamosa_ck_media_management_search_wrapper(),
      ),
    ),
    'tab-overlay' => 'search',
  );

  return theme('mediamosa_ck_tabs', $variables);
}

// ---------------------------------------------------------------- Search form.
/**
 * Search form content.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @return array
 *   The search form.
 */
function mediamosa_ck_media_management_search_form($form, &$form_state) {

  $form['filter'] = array(
    '#prefix' => '<div id="mediamosa-ck-media-management-search-wrapper">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',
    '#title' => 'Search filter',
  );

  // @todo: Get collections.
  $collections = MediaMosaCkConnectorWrapper::get_collections_for_options(array(), array('' => t('Select collection')));

  // Get the metadata options array for dropdown.
  $metadata_options = mediamosa_ck::get_metadata_fields_options();

  // Get the values.
  $values = empty($_SESSION['mediamosa_ck_mm']['search']['values']) ? array() : $_SESSION['mediamosa_ck_mm']['search']['values'];

  // Set default.
  $values += array(
    'search_operator' => 'and',
    'cql' => '',
    'search_collection' => 0,
  );

  $form['filter']['search_operator'] = array(
    '#type' => 'select',
    '#title' => t('Search criteria operator'),
    '#description' => t("Filter on all search criteria ('AND') or a least one criterium ('OR')."),
    '#options' => array('or' => t('OR'), 'and' => t('AND')),
    '#default_value' => $values['search_operator'],
  );

  if (!empty($collections)) {
    $form['filter']['search_options'] = array(
      '#type' => 'fieldset',
      '#title' => 'Search options',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $form['filter']['search_options']['search_published'] = array(
      '#type' => 'select',
      '#title' => t('Published'),
      '#description' => t('limit published / unpublished assets.'),
      '#options' => array(1 => 'show published/unpublished', 2 => 'only published'),
      '#default_value' => 1,
    );

    $form['filter']['search_options']['search_collection'] = array(
      '#type' => 'select',
      '#title' => t('Collection'),
      '#description' => t('Search within the selected collection.'),
      '#options' => $collections,
      '#default_value' => $values['search_collection'],
    );
  }
  else {
    $form['search_collection'] = array('#type' => 'value', '#value' => 0);
  }

  $form['filter']['cql'] = array(
    '#type' => 'fieldset',
    '#title' => 'CQL',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE, //empty($values['cql']),
  );

  $form['filter']['cql']['cql'] = array(
    '#type' => 'textfield',
    '#title' => t('CQL string'),
    '#description' => t('Specify CQL search string to search.'),
    '#default_value' => $values['cql'],
  );

  // Metadata search.
  $form['filter']['metadata'] = array(
    '#type' => 'fieldset',
    '#title' => 'Metadata search',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE, //empty($_SESSION['mediamosa_ck_mm'][$values['type']]['search_metadata']),
  );

  // Get metadata def.
  $metadata_definition = mediamosa_ck::get_metadata_fields();

  if (!empty($_SESSION['mediamosa_ck_mm']['search']['search_metadata'])) {
    foreach ($_SESSION['mediamosa_ck_mm']['search']['search_metadata'] as $prop => $items) {
      // Prop is [set].[name]
      list($set, $name) = explode('.', $prop);
      $title = mediamosa_ck::get_metadata_label($prop) . ' (' . t($metadata_definition[$set]['title']) . ')';
      foreach (array_keys($items) as $key_item) {
        // Build id.
        $id = $set . '-' . $name . '-' . $key_item;

        // Get possible value.
        switch ($metadata_definition[$set]['fields'][$name]) {
          case 'CHAR':
          case 'INT':
            $value = isset($values[$id]) && $values[$id] !== '' ? $values[$id] : NULL;
            $form['filter']['metadata'][$id] = array(
              '#type' => 'textfield',
              '#title' => check_plain($title),
              '#default_value' => isset($value) ? $value : '',
            );
            break;

          case 'DATETIME':
            $id_from = $id;
            $id_till = $id . '-till';
            $value_from = isset($values[$id_from]) && $values[$id_from] !== '' ? $values[$id_from] : NULL;
            $value_till = isset($values[$id_till]) && $values[$id_till] !== '' ? $values[$id_till] : NULL;

            $form['filter']['metadata']['date-' . $id] = array(
              '#prefix' => '<div class="ck-cs-date-fieldset">',
              '#suffix' => '</div>',
              '#type' => 'fieldset',
              '#title' => $title,
              '#description' => t('Search from date (YYYY[-MM-DD HH:MM:SS]) till date (YYYY[-MM-DD HH:MM:SS]). Till is optional.'),
            );
            $form['filter']['metadata']['date-' . $id][$id] = array(
              '#prefix' => '<div class="ck-cs-date-from">',
              '#suffix' => '</div>',
              '#title' => t('From'),
              '#type' => 'textfield',
              '#default_value' => $value_from,
              '#size' => 12,
            );

            $form['filter']['metadata']['date-' . $id][$id . '-till'] = array(
              '#prefix' => '<div class="ck-cs-date-till">',
              '#suffix' => '</div>',
              '#title' => t('Till (optional)'),
              '#type' => 'textfield',
              '#default_value' => $value_till,
              '#size' => 12,
            );
            break;
        }
      }
    }
  }

  $form['filter']['metadata']['search_metadata_prop'] = array(
    '#type' => 'select',
    '#title' => t('Metadata property'),
    '#options' => $metadata_options,
    '#default_value' => 'dc.title',
    '#size' => 1,
  );

  $form['filter']['metadata']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add search item'),
    '#ajax' => array(
      'callback' => '_mediamosa_ck_media_management_search_ajax',
      'wrapper' => 'mediamosa-ck-media-management-search-wrapper',
      'method' => 'replace',
      'event' => 'click',
      'progress' => array('message' => ''),
    ),
  );

  $form['filter']['mediamosa_ck_media_management_search'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
    '#ajax' => array(
      'callback' => '_mediamosa_ck_media_management_search_ajax',
      'wrapper' => 'mediamosa-ck-media-management-search-result-wrapper',
      'method' => 'replace',
      'event' => 'click',
      'progress' => array('message' => ''),
    ),
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'mediamosa_ck_media_management') . '/mediamosa_ck_media_management.js',
      ),
    ),
  );

  $form['filter']['reset'] = array(
    '#type' => 'button',
    '#value' => t('Reset'),
    '#ajax' => array(
      'callback' => '_mediamosa_ck_media_management_search_ajax',
      'wrapper' => 'mediamosa-ck-media-management-search-wrapper',
      'method' => 'replace',
      'event' => 'click',
      'progress' => array('message' => ''),
    ),
  );

  return $form;
}

/**
 * Implements hook_validate().
 */
function mediamosa_ck_media_management_search_form_validate($form, $form_state) {
}

/**
 * Implements hook_submit().
 */
function mediamosa_ck_media_management_search_form_submit($form, &$form_state) {
}

/**
 * Ajax callback for search form.
 */
function _mediamosa_ck_media_management_search_ajax($form, &$form_state) {
  // Process the button click.
  $op = $form_state['values']['op'];

  switch ($op) {
    case t('Search'):
      // Need to store possible changes.
      $_SESSION['mediamosa_ck_mm']['search']['values'] = $form_state['input'];

      $commands = array();
      $commands[] = ajax_command_html(NULL, _mediamosa_ck_media_management_search_page());
      return array('#type' => 'ajax', '#commands' => $commands);

    case t('Reset'):
      unset($_SESSION['mediamosa_ck_mm']['search']);
      break;

    case t('Add search item'):
      // Need to store possible changes.
      $_SESSION['mediamosa_ck_mm']['search']['values'] = $form_state['input'];

      // Adding new item.
      $_SESSION['mediamosa_ck_mm']['search']['search_metadata'][$form_state['values']['search_metadata_prop']][] = array();
      break;
  }

  // Get our search form.
  $form = drupal_rebuild_form('mediamosa_ck_media_management_search_form', $form_state);

  // This ID must not be changed for js, else start submit wlll.
  $form['filter']['mediamosa_ck_media_management_search']['#id'] = 'edit-mediamosa-ck-media-management-search';

  $commands = array();
  $commands[] = ajax_command_html(NULL, drupal_render($form['filter']));
  return array('#type' => 'ajax', '#commands' => $commands);
}

// -------------------------------------------------------------- Search result.
/**
 * Generate the search result table.
 *
 * @return string
 *   The search result table.
 */
function _mediamosa_ck_media_management_search_wrapper() {
  return '<div id="mediamosa-ck-media-management-search-result-wrapper">' . _mediamosa_ck_media_management_search_page() . '</div>';
}

/**
 * Generate the search result table.
 *
 * @return string
 *   The search result table.
 */
function _mediamosa_ck_media_management_search_page() {

  global $user;

  $page = isset($_GET['page']) ? $_GET['page'] : 0;
  $limit = mediamosa_ck_media_management::get_setting_max_items_search_result_list();
  $offset = $page * $limit;

  // Setup header.
  $header = array(
    'title' => t('Title'),
    'isprivate' => t('Published'),
    'videotimestampmodified' => t('Modified'),
    t('Actions'),
  );

  $options = array(
    'offset' => $offset,
    'limit' => $limit,
  );

  // Search for assets.
  $assets = mediamosa_ck_media_management::search_assets($options);
  $item_count_total = $assets['item_count_total'];

  // Get default checks.
  $default_value = array();
  foreach (mediamosa_ck_media_management::get_selection_state($user->uid, array_keys($assets['assets'])) as $asset_id) {
    $default_value[$asset_id] = 1;
  }

  $rows = array();
  foreach ($assets['assets'] as $asset_id => $asset) {
    $actions = array();
    $actions[] = l(t('view'), request_path() . '/asset/' . $asset_id, array('query' => drupal_get_destination(), 'attributes' => array('class' => array('cs-link-popup'))));

    $rows[$asset_id] = array(
      array(
        'data' => $asset['title'],
      ),
      array(
        'data' => $asset['isprivate'] == 'TRUE' ? 'no' : 'yes',
      ),
      array(
        'data' => $asset['videotimestampmodified'],
      ),
      array(
        'data' => empty($actions) ? '-' : implode(', ', $actions),
      ),
    );
  }

  return theme(
    'mediamosa_ck_table',
    array(
      'header' => $header,
      'rows' => $rows,
      'empty' => t('There are no assets found.'),
      'attributes' => array('class' => array('mediamosa-ck-media-management-search-result')),
      'item_count_total' => $item_count_total,
      'item_limit' => $limit,
      'page' => $page,
      'pager top' => FALSE,
      'selection' => $default_value,
      'selection checkbox class' => 'mm-ck-cs-checkbox-search',
      'caption_bottom' => t('Total found: @total', array('@total' => $item_count_total)),
    )
  );
}

/**
 * Called from JQuery when checkbox is (un)checked in search result.
 *
 * @param string $id
 *   The asset ID.
 * @param string $state
 *   Either 'true' or 'false'. State of the checkbox.
 */
function _mediamosa_ck_json_mediamanagement_selection_state_ajax($id, $state) {
  global $user;
  mediamosa_ck_media_management::set_selection_state($user->uid, $id, drupal_strtolower($state) == 'true');
  drupal_exit();
}

// -------------------------------------------------------- Batch panel + forms.
/**
 * Process panel.
 *
 * @return string
 *   The batch panel in HTML.
 */
function _mediamosa_ck_media_management_batch_panel() {
  $variables = array(
    'tabs' => array(
      array(
        'title' => 'Selection',
        'content' => _mediamosa_ck_media_management_batch_selection_form_page(),
      ),
      array(
        'title' => 'Create batch',
        'content' => _mediamosa_ck_media_management_batch_create_form_page(),
      ),
      array(
        'title' => 'Batches',
        'content' => _mediamosa_ck_media_management_batch_listing_page(),
      ),
    ),
    'tab-overlay' => 'batch',
  );

  // If there batches in the queue, we include the queue listing.
  if (mediamosa_ck_media_management_queue::count_queued_batches()) {
    $variables['tabs'][] = array(
      'title' => t('Queue'),
      'content' => _mediamosa_ck_media_management_batch_panel_queue_page(),
    );
  }

  return theme('mediamosa_ck_tabs', $variables);
}

/**
 * Returns the content of the batch selection.
 *
 * @return string
 *   The batch selection content.
 */
function _mediamosa_ck_media_management_batch_selection_form_page() {
  // Get the batch selection form.
  $batch_selection_form = drupal_get_form('mediamosa_ck_media_management_batch_selection_form');
  return drupal_render($batch_selection_form);
}

/**
 * Returns the content of the batch creation form.
 *
 * @return string
 *   The batch creation form.
 */
function _mediamosa_ck_media_management_batch_create_form_page() {
  // Get the batch selection form.
  $batch_create_form = drupal_get_form('mediamosa_ck_media_management_batch_create_form');
  return drupal_render($batch_create_form);
}

/**
 * Returns the content of the queue listing.
 *
 * @return string
 *   The queue listing content.
 */
function _mediamosa_ck_media_management_batch_panel_queue_page() {
  $queued = drupal_get_form('mediamosa_ck_media_management_batch_panel_queue_form');
  return drupal_render($queued);
}

/**
 * Returns the queue form.
 *
 * @return array
 *   The queue form.
 */
function mediamosa_ck_media_management_batch_panel_queue_form() {
  $form['queued'] = array(
    '#type' => 'fieldset',
    '#title' => t('Queued batches'),
  );

  $form['queued']['queue'] = array(
    '#theme' => 'mediamosa_ck_tabs',
    '#tabs' => array(
      array(
        'title' => 'Queued',
        'content' => _mediamosa_ck_media_management_queue_listing_wrapper(),
      ),
      array(
        'title' => 'Done',
        'content' => _mediamosa_ck_media_management_queue_done_listing_wrapper(),
      ),
    ),
    '#tab-overlay' => 'queue',
  );

  return $form;
}

/**
 * Returns the html for the plugin form.
 */
function _mediamosa_ck_media_management_batch_queue_form_ajax() {
  $form = array();
  $form_state = array();
  $form_state['build_info']['args'] = func_get_args();
  print drupal_render(mediamosa_ck_media_management_batch_queue_form($form, $form_state));
  drupal_exit();
}

/**
 * The creation batch form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @return array
 *   The form.
 */
function mediamosa_ck_media_management_batch_create_form($form, $form_state) {

  $form['batch'] = array(
    '#type' => 'fieldset',
    '#title' => t('Batch creation'),
  );
  $form['batch']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => '',
    '#description' => t('The title of the batch.'),
    '#required' => TRUE,
  );
  $form['batch']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Short description'),
    '#default_value' => '',
    '#rows' => 4,
    '#maxlength' => mediamosa_ck_media_management::BATCH_MAX_DESCRIPTION_LENGTH,
    '#description' => t('Short description about the batch contents.'),
  );
  $form['batch']['submit'] = array('#type' => 'submit', '#value' => t('Create batch'));

  return $form;
}

/**
 * Implements hook_validate().
 */
function mediamosa_ck_media_management_batch_create_form_validate($form, $form_state) {
  // Get number of selected assets.
  if (!mediamosa_ck_media_management::get_batch_asset_count()) {
    form_set_error('', t('You must select some assets first before you can create a batch.'));
  }
}

/**
 * Implements hook_submit().
 */
function mediamosa_ck_media_management_batch_create_form_submit($form, $form_state) {

  // Get the values.
  $values = $form_state['values'];

  if ($values['op'] == t('Create batch')) {
    mediamosa_ck_media_management::create_batch_from_selection($values['title'], $values['description']);

    // Done.
    drupal_set_message(t('Batch %title created', array('%title' => $values['title'])));

    // Show the batch list tab.
    drupal_goto(request_path(), array('fragment' => 'ck-tab-batches'));
  }
}

/**
 * Form wrapper for batch selection.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @return string
 *   The HTML.
 */
function _mediamosa_ck_media_management_batch_selection_wrapper($form, $form_state) {
  return '<div id="mediamosa-ck-media-management-batch-selection-wrapper">' . _mediamosa_ck_media_management_batch_selection_page() . '</div>';
}

/**
 * Return the batch selection element rendered.
 *
 * @return string
 *   The HTML.
 */
function _mediamosa_ck_media_management_batch_selection_page() {
  // Setup header.
  $header = array(
    'title' => t('Title'),
    'isprivate' => t('Published'),
    'videotimestampmodified' => t('Modified'),
  );

  $page = isset($_GET['page']) ? $_GET['page'] : 0;
  $limit = mediamosa_ck_media_management::get_setting_max_items_process_selection_list();
  $offset = $page * $limit;
  $options = array(
    'offset' => $offset,
    'limit' => $limit,
  );
  $assets = mediamosa_ck_media_management::get_selection_assets($options);
  $item_count_total = mediamosa_ck_media_management::get_batch_asset_count();

  $options = array();
  $default_value = array();
  if (!empty($assets)) {
    foreach ($assets['assets'] as $asset_id => $asset) {
      $options[$asset_id] = array(
        'title' => $asset['title'],
        'isprivate' => $asset['isprivate'] == 'TRUE' ? 'no' : 'yes',
        'videotimestampmodified' => $asset['videotimestampmodified'],
      );

      // By default, all are selected.
      $default_value[$asset_id] = 1;
    }
  }

  return theme('mediamosa_ck_table',
    array(
      'attributes' => array('class' => array('mediamosa-ck-media-management-batch-selection')),
      'header' => $header,
      'rows' => $options,
      'empty' => t('There are no assets selected.'),
      'selection' => $default_value,
      'pager top' => FALSE,
      'page' => $page,
      'item_count_total' => $item_count_total,
      'item_limit' => $limit,
    )
  );
}

/**
 * Generate the batch selection form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @return array
 *   Form array to render.
 */
function mediamosa_ck_media_management_batch_selection_form($form, &$form_state) {

  $form['mediamosa_ck_media_management']['mediamosa_ck_media_management_batch_selection'] = array(
    '#prefix' => '<div id="mediamosa-ck-media-management-batch-selection-wrapper">',
    '#suffix' => '</div>',
    '#markup' => _mediamosa_ck_media_management_batch_selection_page(),
  );
  $form['mediamosa_ck_media_management']['mediamosa_ck_media_management_batch_selection_refresh'] = array(
    '#type' => 'button',
    '#value' => t('Refresh'),
    '#ajax' => array(
      'callback' => '_mediamosa_ck_media_management_batch_selection_wrapper',
      'wrapper' => 'mediamosa-ck-media-management-batch-selection-wrapper',
      'method' => 'replace',
      'effect' => 'fade',
      'speed' => 'fast',
      'event' => 'click',
      'progress' => array('message' => ''),
    ),
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'mediamosa_ck_media_management') . '/mediamosa_ck_media_management.js',
      ),
    ),
  );

  return $form;
}

/**
 * Generate the batch listing form.
 *
 * Shows an listing of the current created batches.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @return array
 *   Form array to render.
 */
function _mediamosa_ck_media_management_batch_listing_page() {
  // Setup header.
  $header = array(
    'title' => t('Title'),
    'count' => t('# of assets'),
    'created' => t('Created'),
    'status' => t('Status'),
    'actions' => t('Actions'),
  );

  $options = array(
    'offset' => 0,
  );
  $batches = mediamosa_ck_media_management::get_batches($options);

  $rows = array();
  foreach ($batches as $batch) {
    $actions = array();
    $actions[] = l(t('delete'), request_path() . '/batch/' . $batch['bid'] . '/delete', array('query' => drupal_get_destination(), 'attributes' => array('class' => array('cs-link-overlay-batch'))));
    $actions[] = l(t('queue'), request_path() . '/batch/' . $batch['bid'] . '/queue', array('query' => drupal_get_destination(), 'attributes' => array('class' => array('cs-link-overlay-batch'))));

    $rows[] = array(
      check_plain($batch['title']),
      $batch['total'],
      format_date($batch['created']),
      'Not in queue',
      empty($actions) ? '-' : implode(', ', $actions),
    );
  }

  return theme(
    'mediamosa_ck_table',
    array(
      'header' => $header,
      'rows' => $rows,
      'empty' => t('There are no batches created.'),
      'attributes' => array('class' => array('mediamosa-ck-cs-batch-listing'))));
}

/**
 * Form to confirm batch delete operation.
 */
function mediamosa_ck_media_management_batch_delete_confirm($form, &$form_state, $bid) {
  // Get the batch.
  $batch = mediamosa_ck_media_management::get_batch($bid);

  $destination = drupal_get_destination();
  $form['bid'] = array(
    '#type' => 'value',
    '#value' => $bid,
  );
  $form['destination'] = array(
    '#type' => 'value',
    '#value' => reset($destination),
  );

  $form['confirm'] = array(
    '#type' => 'fieldset',
    '#title' => t('Delete batch %name', array('%name' => $batch['title'])),
  );

  $text = array();
  $text[] = t('Are you sure you want to delete batch %name ?', array('%name' => $batch['title']));
  $text[] = t('This action cannot be undone.');

  $form['confirm']['form'] = confirm_form(
    $form,
    t('Are you sure you want to delete batch %name ?', array('%name' => $batch['title'])),
    reset($destination),
    '<p>' . implode('</p><p>', $text) . '</p>',
    t('Delete')
  );

  // Check if we can delete.
  if (!mediamosa_ck_media_management_queue::can_delete_batch($bid)) {
    $form['confirm']['form']['description'] = array(
      '#markup' => t('The batch %name can not be deleted, its being used.', array('%name' => $batch['title'])),
    );

    // Remove the submit button.
    unset($form['confirm']['form']['actions']['submit']);
  }


  return $form;
}

/**
 * Form submit handler for mediamosa_ck_media_management_batch_delete_confirm().
 */
function mediamosa_ck_media_management_batch_delete_confirm_submit($form, &$form_state) {
  mediamosa_ck_media_management::delete_batch((int) $form_state['values']['bid']);
  drupal_set_message(t('The batch has been deleted.'));
  $form_state['redirect'] = $form_state['values']['destination'];
}

/**
 * Implements hook_form().
 */
function mediamosa_ck_media_management_batch_queue_form($form, &$form_state, $bid) {
  // Get the batch.
  $batch = mediamosa_ck_media_management::get_batch($bid);

  $form['bid'] = array(
    '#type' => 'value',
    '#value' => $bid,
  );
  $form['queue'] = array(
    '#type' => 'fieldset',
    '#title' => t('Queue batch - %title', array('%title' => $batch['title'])),
    '#description' =>
      '<p>' .
      t("You have selected batch '%title' for queue. It has @total of asset(s) in the batch and was created on %date. Batch description; '%description'.",
        array(
          '%title' => $batch['title'],
          '@total' => $batch['total'],
          '%date' => format_date($batch['created'], 'long'),
          '%description' => $batch['description'],
        )
      ) .
      '</p><p>' .
      t('Select the batch operation and fill in the batch form to queue the batch.') .
      '</p>',
    '#prefix' => '<div id="mediamosa-ck-media-management-plugin-form-wrapper">',
    '#suffix' => '</div>',
  );

  $form['queue']['plugin'] = array(
    '#type' => 'select',
    '#title' => t('The batch operation'),
    '#required' => TRUE,
    '#options' => mediamosa_ck_media_management::get_plugin_infos_options(),
    '#ajax' => array(
      'callback' => '_mediamosa_ck_media_management_plugin_form_wrapper',
      'wrapper' => 'mediamosa-ck-media-management-plugin-form-wrapper',
      'method' => 'replace',
      'event' => 'change',
      'progress' => array('message' => ''),
    ),
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'mediamosa_ck_media_management') . '/mediamosa_ck_media_management.js',
      ),
    ),
  );

  if (!empty($form_state['values']['plugin'])) {
    $plugin = $form_state['values']['plugin'];
    $plugins = mediamosa_ck_media_management::get_plugin_infos();

    if (isset($plugins[$plugin]) && isset($plugins[$plugin]['class'])) {
      $form['queue']['plugin_form'] = array(
        '#type' => 'fieldset',
        '#title' => t('@plugin_title settings', array('@plugin_title' => $plugins[$plugin]['title'])),
      );

      $form['queue']['plugin_form']['form'] = $plugins[$plugin]['class']::settings_form(array(), $form_state, $batch);
    }

    $form['queue']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Queue batch'),
    );
  }

  return $form;
}

/**
 * Implements hook_validate().
 */
function mediamosa_ck_media_management_batch_queue_form_validate($form, $form_state) {
  if (!empty($form_state['values']['plugin'])) {
    $plugin = $form_state['values']['plugin'];
    $plugins = mediamosa_ck_media_management::get_plugin_infos();

    if (isset($plugins[$plugin]) && isset($plugins[$plugin]['class'])) {
      // Get batch ID.
      $bid = $form_state['values']['bid'];

      // Get the batch.
      $batch = mediamosa_ck_media_management::get_batch($bid);

      $plugins[$plugin]['class']::settings_form_validate($form, $form_state, $batch);
    }
  }
}

/**
 * Implements hook_submit().
 */
function mediamosa_ck_media_management_batch_queue_form_submit($form, $form_state) {
  global $user;

  $plugin = $form_state['values']['plugin'];
  $plugins = mediamosa_ck_media_management::get_plugin_infos();

    // Get batch ID.
  $bid = $form_state['values']['bid'];

  if (isset($plugins[$plugin]) && isset($plugins[$plugin]['class'])) {
    // Get batch ID.
    $bid = $form_state['values']['bid'];

    // Get the batch.
    $batch = mediamosa_ck_media_management::get_batch($bid);

    $plugins[$plugin]['class']::settings_form_submit($form, $form_state, $batch);
  }

  // No need for these.
  unset(
    $form_state['values']['form_build_id'],
    $form_state['values']['form_token'],
    $form_state['values']['plugin']
  );

  // Store the queue batch.
  $queue_batch = array(
    'uid' => $user->uid,
    'bid' => $bid,
    'created' => REQUEST_TIME,
    'queue_plugin_id' => $plugin,
    'queue_plugin_title' => $plugins[$plugin]['title'],
    'queue_plugin_class' => $plugins[$plugin]['class'],
    'queue_plugin_data' => $form_state['values'],
  );

  // Store it.
  mediamosa_ck_media_management_queue::store_batch_queue($queue_batch);

  drupal_set_message(t('The batch has been inserted into the queue.'));

  // Show the batch list tab.
  drupal_goto(request_path(), array('fragment' => 'ck-tab-queue'));
}

/**
 * Show the log of a batch run.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 * @param integer $qid
 *   The queued batch ID.
 *
 * @return string
 *   The HTML.
 */
function mediamosa_ck_media_management_batch_queued_log_form($form, &$form_state, $qid) {

  $classes = array(
    WATCHDOG_DEBUG     => 'dblog-debug',
    WATCHDOG_INFO      => 'dblog-info',
    WATCHDOG_NOTICE    => 'dblog-notice',
    WATCHDOG_WARNING   => 'dblog-warning',
    WATCHDOG_ERROR     => 'dblog-error',
    WATCHDOG_CRITICAL  => 'dblog-critical',
    WATCHDOG_ALERT     => 'dblog-alert',
    WATCHDOG_EMERGENCY => 'dblog-emerg',
  );

  $limit = mediamosa_ck_media_management::get_setting_max_items_log_list();
  $header = array(
    // Icon column.
    '',
    // @todo: Needs ajax love to sort on same screen.
    array('data' => t('Date')/*, 'field' => 'log.lid', 'sort' => 'asc'*/),
    t('Message'),
  );

  $result = db_select('mediamosa_ck_mm_batch_queue_log', 'log')->extend('PagerDefault')->extend('TableSort')
    ->fields('log', array('lid', 'severity', 'timestamp', 'message', 'variables'))
    ->condition('qid', $qid)
    ->limit($limit)
    ->orderByHeader($header)
    ->execute();

  $rows = array();
  foreach ($result as $dblog) {
    $rows[] = array('data' =>
      array(
        // Cells
        array('class' => 'icon'),
        format_date($dblog->timestamp, 'short'),
        theme('dblog_message', array('event' => $dblog, 'link' => FALSE)),
      ),
      // Attributes for tr
      'class' => array($classes[$dblog->severity]),
    );
  }

  // Need the batch info to display.
  // Get batch ID.
  $batch_queue = mediamosa_ck_media_management_queue::get_batch_queue($qid);

  // Get batch.
  $batch = mediamosa_ck_media_management::get_batch($batch_queue['bid']);

  $form['log'] = array(
    '#theme' => 'mediamosa_ck_mm_log',
    '#title' => $batch['title'],
    '#description' => t('@descriptionThere are @count log entries.', array('@count' => $batch_queue['total'], '@description' => ($batch['description'] == '' ? '' : t('@description. ', array('@description' => rtrim($batch['description'], '.')))))),
    '#prefix' => '<div id="mediamosa-ck-media-management-log-wrapper">',
    '#suffix' => '</div>',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No log messages available.'),
  );

  return $form;
}

/**
 * Return a wrapped queue listing.
 *
 * @return string
 *   The HTML.
 */
function _mediamosa_ck_media_management_queue_listing_wrapper() {
  return '<div id="mediamosa-ck-media-management-queue-listing-wrapper">' . _mediamosa_ck_media_management_queue_listing_page(array('state_start' => mediamosa_ck_media_management_queue::STATE_START, 'state_end' => mediamosa_ck_media_management_queue::STATE_RUNNING_END)) . '</div>';
}

/**
 * Return a wrapped queue listing.
 *
 * @return string
 *   The HTML.
 */
function _mediamosa_ck_media_management_queue_done_listing_wrapper() {
  return '<div id="mediamosa-ck-media-management-queue-done-listing-wrapper">' . _mediamosa_ck_media_management_queue_listing_page(array('state_start' => mediamosa_ck_media_management_queue::STATE_DONE, 'state_end' => mediamosa_ck_media_management_queue::STATE_END)) . '</div>';
}


/**
 * Generate the queue listing form.
 *
 * Shows an listing of the current queued batches.
 *
 * @return string
 *   The HTML.
 */
function _mediamosa_ck_media_management_queue_listing_page(array $options = array()) {
  // Setup header.
  $header = array(
    'batch' => t('Batch'),
    'title' => t('Plugin'),
    'count' => t('# of assets'),
    'created' => t('Created'),
    'status' => t('Status'),
    'actions' => t('Actions'),
  );

  $limit = mediamosa_ck_media_management::get_setting_max_items_queued_batches_list();
  $page = isset($_GET['page']) ? $_GET['page'] : 0;
  $offset = $page * $limit;
  $options += array(
    'offset' => $offset,
    'limit' => $limit,
  );
  $queued_batches = mediamosa_ck_media_management_queue::get_queued_batches($options);

  $rows = array();
  foreach ($queued_batches as $queued_batch) {
    $actions = array();
    $actions[] = l(t('log'), request_path() . '/batch/queued/' . $queued_batch['qid'] . '/log', array('query' => array('destination' => drupal_get_destination()), 'attributes' => array('class' => array('cs-link-popup'))));

    $rows[] = array(
      check_plain(truncate_utf8($queued_batch['title'], 40, TRUE, TRUE)),
      check_plain(truncate_utf8($queued_batch['queue_plugin_title'], 40, TRUE, TRUE)),
      $queued_batch['total'],
      format_date($queued_batch['created']),
      mediamosa_ck_media_management_queue::get_state_text($queued_batch['state'], $queued_batch['position'], $queued_batch['total']),
      empty($actions) ? '-' : implode(', ', $actions),
    );
  }

  return theme(
    'mediamosa_ck_table',
    array(
      'header' => $header,
      'rows' => $rows,
      'empty' => t('There are no batches queued.'),
      'attributes' => array('class' => array('mediamosa-ck-cs-batch-listing')),
      'item_count_total' => mediamosa_ck_media_management_queue::count_queued_batches($options),
      'item_limit' => $limit,
      'page' => $page,
      'pager top' => FALSE,
    )
  );
}

/**
 * Process the incoming ajax call for paging.
 *
 * @return array
 *   The result ajax command array.
 */
function _mediamosa_ck_mm_ajax_paging() {
  if (!isset($_REQUEST['wrapper']) || !isset($_REQUEST['path'])) {
    return;
  }

  // Get the wrapper.
  $wrapper = $_REQUEST['wrapper'];
  $path = trim($_REQUEST['path'], '/');

  // Remove all of this stuff from $_GET so it doesn't end up in pagers and tablesort URLs.
  foreach (array('wrapper', 'path', 'ajax_html_ids', 'ajax_page_state') as $key) {
    if (isset($_GET[$key])) {
      unset($_GET[$key]);
    }
    if (isset($_REQUEST[$key])) {
      unset($_REQUEST[$key]);
    }
    if (isset($_POST[$key])) {
      unset($_POST[$key]);
    }
  }

  // Add all $_POST data, because AJAX is always a post and many things,
  // such as tablesorts, exposed filters and paging assume $_GET.
  $_GET = $_POST + $_GET;

  // Fix q.
  $_GET['q'] = $path;

  switch ($wrapper) {
    case 'mediamosa-ck-media-management-search-result-wrapper':
      $content = _mediamosa_ck_media_management_search_wrapper();
      break;

    case 'mediamosa-ck-media-management-batch-selection-wrapper':
      $content = _mediamosa_ck_media_management_batch_selection_wrapper();
      break;

    case 'mediamosa-ck-media-management-queue-listing-wrapper':
      $content = _mediamosa_ck_media_management_queue_listing_wrapper();
      break;

    case 'mediamosa-ck-media-management-queue-done-listing-wrapper':
      $content = _mediamosa_ck_media_management_queue_done_listing_wrapper();
      break;

    case 'mediamosa-ck-media-management-log-wrapper':
      $form = drupal_get_form('mediamosa_ck_media_management_batch_queued_log_form', arg(6, $path));
      $wrapper = 'mediamosa-ck-cs-popup-full';
      $content = '<div id="mediamosa-ck-cs-popup-full">' . drupal_render($form) . '</div>';
      break;

    // Not known, fail.
    default:
      return;
  }

  $commands[] = ajax_command_replace('#' . $wrapper, $content);
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Form wrapper for batch selection.
 *
 * @param integer $asset_id
 *   The asset ID.
 *
 * @return string
 *   The HTML.
 */
function _mediamosa_ck_media_management_search_item_view_page($asset_id) {
  $form = drupal_get_form('mediamosa_ck_media_management_asset_view_form', $asset_id);
  return theme('mediamosa_ck_popup', array('markup' => drupal_render($form)));
}

/**
 * Show the log of a batch run.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 * @param integer $asset_id
 *   The asset ID.
 *
 * @return string
 *   The HTML.
 */
function mediamosa_ck_media_management_asset_view_form($form, &$form_state, $asset_id = null) {

  if (empty($asset_id)) {
    $asset_id = $form_state['build_info']['args'][0];
  }

  $form = array();
  $form['asset'] = array(
    '#prefix' => '<div id="mediamosa-ck-media-management-asset-view">',
    '#suffix' => '</div>',
    '#type' => 'vertical_tabs',
  );
  $form['asset']['asset_id'] = array(
    '#value' => $asset_id,
  );

  try {
    $response = MediaMosaCkConnectorWrapper::get_asset($asset_id);
  }
  catch(Exception $e) {
    mediamosa_ck::set_message_exception($e, t('Unable to get asset.'));
    return '';
  }

  $asset = $response->get_asset();
  $title = mediamosa_ck::get_asset_title($asset, t('No title available.'));

  $metadata = mediamosa_ck::get_metadata_asset($asset, array());

  $form['asset-info'] = array(
    '#type' => 'fieldset',
    '#title' => 'info',
  );

  foreach ($metadata as $set => $data) {
    $form['asset-metadata-' . $set] = array(
      '#type' => 'fieldset',
      '#title' => $data['title'],
      '#collapsed' => TRUE,
    );
    foreach ($data['data'] as $name => $contents) {
      $markup = array();
      if (!empty($contents['contents'])) {
        foreach ($contents['contents'] as $content) {
          $markup[] = check_plain($content);
        }
      }
      $form['asset-metadata-' . $set][$name] = array(
        '#theme' => 'mediamosa_ck_mm_name_value_pair',
        '#title' => $contents['label'],
        '#default_value' => implode('<br \>', $markup),
      );
    }
  }

  $form['asset-info']['title'] = array(
    '#markup' => $title,
  );

  foreach ($form as $name => $item) {
    if (empty($item['#type']) || $item['#type'] != 'fieldset') {
      continue;
    }

    // Make sure the fieldset is collapsed.
    $form[$name]['#collapsed'] = TRUE;

    // Add group type to fieldset.
    $form[$name]['#group'] = 'asset';
  }

  return $form;
}

/**
 * Implements hook_submit().
 */
function mediamosa_ck_media_management_asset_view_form_submit($form, &$form_state) {
}
