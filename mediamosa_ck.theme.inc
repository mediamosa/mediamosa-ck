<?php
/**
 * @file
 * The theme functions of CK.
 */

/**
 * Our default CK themed table.
 *
 * @param array $variables
 *   An associative array;
 *   Default table variables;
 *   - 'header'
 *   - 'rows'
 *   - 'attributes'
 *   - 'caption'
 *   - 'colgroups'
 *   - 'sticky'
 *   - 'empty'
 *   - Draggable rows: (reserved)
 *     - 'draggable' => ID (default is FALSE).
 *       Provide ID for table where the Dragable code can identify the table as
 *       dragable. F.e. 'ck-users-list'.
 *       Do not use pagers when dragable as you can not drag from page to page.
 *   Add selection boxes (checkbox).
 *   - 'selection' => array(with selected IDs or empty) | NULL
 *   - 'selection checkbox class' => 'mm-ck-mm-checkbox' (optional).
 *   Pager (optional)
 *   - 'page' => 0
 *   - 'item_count_total' => 0,
 *   - 'item_limit' => 25,
 *   - 'element' => 0,
 *   - 'quantity' => 10,
 *   - 'pager top' => TRUE,
 *     Include pager at top.
 *   - 'pager bottom' => TRUE,
 *     Include pager at bottom.
 *   - 'pager normal top' => FALSE,
 *     Add normal Drupal theme_pager() at top. Expect usage of
 *     ->extend('Pagerdefault') on used query.
 *   - 'pager normal bottom' => FALSE,
 *     Add normal Drupal theme_pager() at bottom. Expect usage of
 *     ->extend('Pagerdefault') on used query.
 *   Note (small font, bottom right).
 *   - 'note' => NULL,
 *   // caption bottom, right.
 *   - 'caption_bottom' => NULL,
 *   // Title, description (top).
 *   - 'title' => NULL,
 *   - 'description' => NULL,
 *   // Summary.
 *   - 'summary' => NULL,
 *
 * @return string
 *   The result HTML.
 */
function theme_mediamosa_ck_table($variables) {

  // Add extra selection box.
  if (isset($variables['selection'])) {
    $all_selected = count($variables['selection']) == count($variables['rows']) && count($variables['rows']);
    $ids = drupal_http_build_query(array_keys($variables['rows']));

    if (!empty($variables['selection checkbox class all'])) {
      $checkbox = array(
        $ids => array(
          '#return_value' => $ids,
          '#checked' => $all_selected,
          '#type' => 'checkbox',
          '#attributes' => array('class' => array($variables['selection checkbox class all'])),
        ),
      );
      array_unshift($variables['header'], array(
        'class' => array($variables['selection checkbox class all']),
        'data' => drupal_render($checkbox),
      ));
    }
    else {
      array_unshift($variables['header'], array('class' => array('ck-tab-select-all-off')));
    }

    // Add checkbox to rows.
    foreach ($variables['rows'] as $id => &$row) {
      $ids = drupal_http_build_query(array(0 => $id));
      $checkbox = array(
        $ids => array(
          '#return_value' => $ids,
          '#checked' => !empty($variables['selection'][$id]),
          '#type' => 'checkbox',
          '#attributes' => array('class' => array($variables['selection checkbox class'])),
        ),
      );
      $box = drupal_render($checkbox);
      array_unshift($row, $box);
    }
  }

  $pager_variables = array();
  foreach (array('page', 'item_count_total', 'item_limit', 'element', 'quantity') as $var) {
    if (isset($variables[$var])) {
      $pager_variables[$var] = $variables[$var];
    }
  }

  if (!empty($pager_variables['item_count_total']) && $variables['pager top']) {
    $build['pager_top'] = theme('mediamosa_ck_pager', $pager_variables);
  }
  elseif (!empty($variables['pager normal top'])) {
    $build['pager_bottom'] = theme('pager');
  }

  if (!empty($variables['title'])) {
    $build['title'] = '<h1 class="mediamosa-ck-table-title">' . check_plain($variables['title']) . '</h1>';
  }

  if (!empty($variables['description'])) {
    $build['description'] = '<p class="mediamosa-ck-table-description">' . check_plain($variables['description']) . '</p>';
  }

  // Show summary.
  if (!empty($variables['summary'])) {
    foreach ($variables['summary'] as $summary_item) {
      if (isset($summary_item['title']) && isset($summary_item['data'])) {
        $variables['rows'][] = array(
          array('data' => $summary_item['title'], 'class' => 'mediamosa-ck-table-title', 'colspan' => count($variables['header'])-1),
          array('data' => $summary_item['data'], 'class' => 'mediamosa-ck-table-data'),
        );
      }
    }
  }

  // Build the table.
  $build['table'] = theme('table', $variables);

  if (!empty($variables['note'])) {
    $build['note'] = '<div class="mediamosa-ck-table-note">' . check_plain($variables['note']) . '</div>';
  }

  if (!empty($variables['caption_bottom'])) {
    $build['caption_bottom'] = '<div class="mediamosa-ck-table-caption-bottom">' . check_plain($variables['caption_bottom']) . '</div>';
  }

  if (!empty($pager_variables['item_count_total']) && $variables['pager bottom']) {
    $build['pager_bottom'] = theme('mediamosa_ck_pager', $pager_variables);
  }
  elseif (!empty($variables['pager normal bottom'])) {
    $build['pager_bottom'] = theme('pager');
  }

  return implode('', $build);
}

/**
 * Our pager wrapper (meant for REST call paging).
 *
 * @param array $variables
 *   An associated array.
 *
 * @return string
 *   The result HTML.
 */
function theme_mediamosa_ck_pager($variables) {

  $total = $variables['item_count_total'];
  $limit = $variables['item_limit'];
  $element = $variables['element'];

  pager_default_initialize($total, $limit, $element);

  return theme('pager', array('quantity' => $variables['quantity'], 'element' => $element));
}

/**
 * Two column page.
 *
 * @param array $variables
 *   An associative array.
 *   - 'left'
 *     The left part in HTML
 *   - 'right'
 *     The right part in HTML.
 *
 * @return string
 *   The result HTML.
 */
function theme_mediamosa_ck_two_column_page($variables) {
  return '<div class="mediamosa-ck-two-column"><div class="mediamosa-ck-two-column-left"><div class="mediamosa-ck-two-column-wrapper">' . $variables['left'] . '</div></div><div class="mediamosa-ck-two-column-right"><div class="mediamosa-ck-two-column-wrapper">' . $variables['right'] . '</div></div></div>';
}

/**
 * Three column page.
 *
 * @param array $variables
 *   An associative array.
 *   - 'left'
 *     The left part in HTML
 *   - 'middle'
 *     The middle part in HTML
 *   - 'right'
 *     The right part in HTML.
 *
 * @return string
 *   The result HTML.
 */
function theme_mediamosa_ck_three_column_page($variables) {
  return '<div class="mediamosa-ck-three-column"><div class="mediamosa-ck-three-column-left"><div class="mediamosa-ck-three-column-wrapper">' . $variables['left'] . '</div></div><div class="mediamosa-ck-three-column-middle"><div class="mediamosa-ck-three-column-wrapper">' . $variables['middle'] . '</div></div><div class="mediamosa-ck-three-column-right"><div class="mediamosa-ck-three-column-wrapper">' . $variables['right'] . '</div></div></div>';
}

/**
 * Show tabified content.
 *
 * @param array $variables
 *   An associative array;
 *   - 'tabs'
 *     An associative array's;
 *     - 'title'
 *       The (non-translated) title of the tab. Title is used for creating an
 *       unique ID / class, do not use t(), will be done during theme code.
 *     - 'title extra'
 *       Attached behind the title with '()' around. F.e. 'Food (3)', where '3'
 *       is provided by 'title extra'.
 *     - 'content'
 *       The content of the tab.
 *   - 'tab-overlay'
 *     Name for the tab-overlay, ID will be ck-tab-overlay-[ID]. Can be used to
 *     load other content into.
 */
function theme_mediamosa_ck_tabs($variables) {
  static $id = 1, $x = 1;

  $tabs = array();
  $tabs_content = array();
  $identifiers = array();

  foreach ($variables['tabs'] as $tab) {
    $tab += array('title' => t('no-title-' . $id), 'content' => t('no data'));

    // Create unique ID for <a> link.
    $identifier = drupal_html_class($tab['title']);
    while (isset($identifiers[$identifier])) {
      $identifier . '-' . $x++;
    }
    $identifiers[$identifier] = $identifier;

    $tab_id = empty($tab['id']) ? 'ck-tab-' . $identifier : $tab['id'];

    $tabs['ck-tab-' . $identifier] = '<li>' . l(t($tab['title']), request_path(), array('fragment' => 'ck-tab-' . $identifier, 'attributes' => array('name' => 'ck-tab-content-' . $identifier, 'id' => $tab_id, 'class' => array('ck-tab-link')))) . '</li>';
    $tabs_content['ck-tab-content-' . $identifier] = '<div>' . $tab['content'] . '</div>';
    $id++;
  }

  $content = array();
  $content[] = '<div class="mediamosa-ck-tabs tabs">';

  $content[] = '<ul class="mediamosa-ck-tabs primary tabs primary-tabs links">';
  foreach ($tabs as $tab) {
    $content[] = $tab;
  }
  $content[] = '</ul>';

  foreach ($tabs_content as $tab_id => $tab_content) {
    $content[] = '<div id="' . $tab_id . '" class="mediamosa-ck-tab-content">' . $tab_content . '</div>';
  }

  // Add div for temporary content.
  if (!empty($variables['tab-overlay'])) {
    $content[] = '<div id="ck-tab-overlay-' . drupal_html_class($variables['tab-overlay']) . '" class="mediamosa-ck-tab-content"></div>';
  }

  $content[] = '</div>';
  return implode('', $content);
}

/**
 * Creates the contents of the popup.
 *
 * @param array $variables
 *   An associative array;
 *   - 'title'
 *     The translated title.
 *   - 'markup'
 *     The contents of the popup.
 *
 * @return string
 *   The HTML.
 */
function theme_mediamosa_ck_popup($variables) {

  $icon_close = theme('image', array('path' => drupal_get_path('module', 'mediamosa_ck') . '/images/close.gif'));

  $frameset['element'] = array(
    '#title' => $variables['title'],
    '#attributes' => array('class' => array('mediamosa-ck-popup')),
    '#value' => $variables['markup'],
  );

  $popup = array();
  $popup[] = '<div class="mediamosa-ck-popup"><div class="mediamosa-ck-icon-close">' . $icon_close . '</div>';
  $popup[] = theme('fieldset', $frameset);
  $popup[] = '</div>';

  return implode('', $popup);
}
