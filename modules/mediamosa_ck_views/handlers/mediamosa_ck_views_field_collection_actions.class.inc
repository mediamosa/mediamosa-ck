<?php
/**
 * @file
 * Views field collections actions.
 */

class mediamosa_ck_views_field_collection_actions extends views_handler_field_xss {

  public function option_definition() {
    $options = parent::option_definition();

    $options['collection_actions_theme'] = array('default' => 'mediamosa_ck_views_theme_collection_actions');
    $options['action_path_edit'] = array('default' => 'collection/edit/[coll_id]');
    $options['action_path_delete'] = array('default' => 'collection/delete/[coll_id]');

    return $options;
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['collection_actions_theme'] = array(
      '#title' => t('Theme function to use'),
      '#type' => 'textfield',
      '#default_value' => $this->options['collection_actions_theme'],
    );
    $form['action_path_edit'] = array(
      '#title' => t('Path of the collection for edit page'),
      '#type' => 'textfield',
      '#default_value' => $this->options['action_path_edit'],
      '#description' => t('Use [coll_id] to fill in the collection ID.'),
    );
    $form['action_path_delete'] = array(
      '#title' => t('Path of the collection for delete page'),
      '#type' => 'textfield',
      '#default_value' => $this->options['action_path_delete'],
      '#description' => t('Use [coll_id] to fill in the collection ID.'),
    );
  }

  /**
   * Get the value that's supposed to be rendered.
   *
   * @param object $values
   *   An object containing all retrieved values.
   * @param string $field
   *   Optional name of the field where the value is stored.
   */
  public function get_value($values, $field = NULL) {
    // Get the data.
    $coll_id = $values->coll_id;

    return theme(
      $this->options['collection_actions_theme'],
      array(
        'coll_id' => $coll_id,
        'action_path_edit' => $this->options['action_path_edit'],
        'action_path_delete' => $this->options['action_path_delete'],
      )
    );
  }


  // The following methods express strong SQLisms in the base field handler
  // class. Override and blank them out.
  public function pre_render(&$values) {
  }

  public function add_additional_fields($fields = NULL) {
    foreach ($fields as $field) {
      $this->aliases[$field] = $this->query->add_field($this->table, $field);
    }
  }

  public function click_sort($order) {
    $this->query->add_orderby($this->table, $this->real_field, $order);
  }

  public function query() {
    $this->field_alias = $this->query->add_field($this->table, $this->real_field);

    // Add in additional fields.
    if (!empty($this->additional_fields)) {
      $this->add_additional_fields($this->additional_fields);
    }
  }
}
