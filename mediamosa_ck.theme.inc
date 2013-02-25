<?php

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
 *   Add selection boxes (checkbox).
 *   - 'selection' => 'checkbox' | NULL
 *   Pager (optional)
 *   - 'page' => 0
 *   - 'item_count_total' => 0,
 *   - 'item_limit' => 25,
 *   - 'element' => 0,
 *   - 'quantity' => 10,
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
  $pager_variables = array();
  foreach (array('page', 'item_count_total', 'item_limit', 'element', 'quantity') as $var) {
    if (isset($variables[$var])) {
      $pager_variables[$var] = $variables[$var];
    }
  }

  if (!empty($pager_variables['item_count_total'])) {
    $build['pager_top'] = theme('mediamosa_ck_pager', $pager_variables);
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

  if (!empty($pager_variables['item_count_total'])) {
    $build['pager_bottom'] = theme('mediamosa_ck_pager', $pager_variables);
  }

  return implode('', $build);
}

/**
 * Our pager wrapper.
 *
 * @param array $variables
 *   An associated array.
 *
 * @return string
 *   The result HTML.
 */
function theme_mediamosa_ck_pager($variables) {
  global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;

  $page = $variables['page'];
  $item_count_total = $variables['item_count_total'];
  $item_limit = $variables['item_limit'];
  $element = $variables['element'];
  $quantity = $variables['quantity'];

  $pager_page_array = array($element => $page);

  // Total items.
  $pager_total_items[$element] = $item_count_total;

  // Number of pagelinks.
  $pager_total[$element] = ceil($item_count_total / $item_limit);

  // Page limit.
  $pager_limits[$element] = $item_limit;

  return theme('pager', array('quantity' => $quantity));
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
 *       The (translated) title of the tab.
 *     - 'content'
 *       The content of the tab.
 */
function theme_mediamosa_ck_tabs($variables) {
  $tabs = array();
  $tabs_content = array();

  $id = 1;
  foreach ($variables['tabs'] as $tab) {
    $tab += array('title' => t('no title'), 'content' => t('no data'));

    $tabs['ck-tab-' . $id] = '<li>' . l($tab['title'], arg(), array('fragment' => 'ck-tab-' . $id, 'attributes' => array('name' => 'ck-tab-' . $id))) . '</li>';
    $tabs_content['ck-tab-' . $id] = '<div>' . $tab['content'] . '</div>';
    $id++;
  }

  $content = array();
  $content[] = '<div class="mediamosa-ck-tabs tabs">';

  $content[] = '<ul id="mediamosa-ck-tabs" class="primary tabs primary-tabs links">';
  foreach ($tabs as $tab) {
    $content[] = $tab;
  }
  $content[] = '</ul>';

  foreach ($tabs_content as $id => $tab_content) {
    $content[] = '<div class="mediamosa-ck-tab" id="' . $id . '">' . $tab_content . '</div>';
  }

  $content[] = '</div>';
  return implode('', $content);
}

/*
<div class="asset-technical-information">
  <ul id="asset-tabs">
    <li>
      <?php print l('Description', 'asset/detail/' . $fields['asset_id']->raw, array('fragment' => 'tab-metadata-dc', 'attributes' => array('name' => 'tab-metadata-dc'))); ?>
    </li>
    <li>
      <?php print l('More info', 'asset/detail/' . $fields['asset_id']->raw, array('fragment' => 'tab-metadata-qdc', 'attributes' => array('name' => 'tab-metadata-qdc'))); ?>
    </li>
    <li>
      <?php print l('Technical info', 'asset/detail/' . $fields['asset_id']->raw, array('fragment' => 'tab-technical-metadata', 'attributes' => array('name' => 'tab-technical-metadata'))); ?>
    </li>
  </ul>

  <div class="tab" id="tab-metadata-dc">
    <h2>Metadata DC</h2>
    <?php print $fields['metadata_dc']->content; ?>
  </div>

  <div class="tab" id="tab-metadata-qdc">
    <h2>Metadata QDC</h2>
    <?php print $fields['metadata_qdc']->content; ?>
  </div>

  <div class="tab" id="tab-technical-metadata">
    <h2>Technical metadata</h2>
    <?php print $fields['technical_metadata']->content; ?>
  </div>
</div>
 */