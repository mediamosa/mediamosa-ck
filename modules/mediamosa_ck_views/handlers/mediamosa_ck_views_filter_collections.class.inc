<?php
/**
 * @file
 * Filter for collection selection in an exposed form.
 */

class mediamosa_ck_views_filter_collections extends mediamosa_ck_views_filter {

  /**
   * Add input to filter data.
   */
  public function query() {
    parent::_cql_exact();
  }

  /**
   * Provide a simple textfield for equality.
   */
  public function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'select',
      '#title' => $this->definition['title'],
      '#options' => array_merge(array('All' => t('- Any -')), MediaMosaCkConnectorWrapper::get_collections_for_options()),
      '#default_value' => $this->value,
    );
  }
}
