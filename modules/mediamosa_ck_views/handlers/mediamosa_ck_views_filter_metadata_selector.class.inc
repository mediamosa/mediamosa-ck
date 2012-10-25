<?php
/**
 * @file
 * Provide a filter where you can chose the metatag to filter on.
 */

class mediamosa_ck_views_filter_metadataSelector extends mediamosa_ck_views_filter {

  /**
   * Add input to filter data.
   */
  public function query() {
    if (!empty($this->value)) {
      foreach($this->value as $value) {
        if(!empty($value['string']) && !empty($value['metadata'])) {
          $cql_or = array();
          $cql_or[] = $value['metadata'] . '="' . addslashes($value['string']) . '"';
          $this->query->add_filter($this->field, count($cql_or) > 1 ? '(' . implode(' OR ', $cql_or) . ')' : reset($cql_or));
        }
      }
    }
  }

  /**
   * Provide a simple textfield for equality.
   */
  public function value_form(&$form, &$form_state) {
    $form['value']['#tree'] = TRUE;

    // Get all metadata fields.
    $options = array();
    $types = mediamosa_ck::get_metadata_fields();
    foreach($types as $type) {
      $options[$type['title']] = array();
      if(!empty($type['fields']) && is_array($type['fields'])) {
        foreach ($type['fields'] as $key => $value) {
          $options[$type['title']][$key] = $key;
        }
      }
    }
    $form['value']['metadata'] = array(
        '#type' => 'select',
        '#title' => t('Type'),
        '#options' => $options,
        '#default_value' => '',
      );
    $form['value']['string'] = array(
      '#type' => 'textfield',
      '#title' => t('Text'),
      '#default_value' => '',
    );
  }
}
