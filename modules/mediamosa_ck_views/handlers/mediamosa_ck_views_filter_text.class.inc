<?php
/**
 * @file
 * Views filter text.
 */

class mediamosa_ck_views_filter_text extends mediamosa_ck_views_filter {

  public function option_definition() {
    $options = parent::option_definition();
    $options['friendly_advanced_search'] = array('default' => FALSE);
    return $options;
  }

  public function options_form(&$form, &$form_state) {
    $form['friendly_advanced_search'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable friendly advanced search'),
      '#description' => t('Allows some additional more friendly expressions to be used
                           next to regular CQL, such as "foo bar" to search for the fixed string.
                           This will be translated to the correct CQL ^foo bar^ query.'),
      '#default_value' => $this->friendly_advanced_search_enabled(),
    );

    parent::options_form($form, $form_state);
  }

  private function friendly_advanced_search_enabled() {
    if (isset($this->options['friendly_advanced_search'])) {
      return $this->options['friendly_advanced_search'];
    }
    return FALSE;
  }

  /**
   * Add input to filter data.
   */
  public function query() {
    $this->field = preg_replace('/_/', '.', $this->field, 1);

    if ($this->friendly_advanced_search_enabled()) {
      if (!empty($this->value)) {
        // replace double quotes by the correct CQL syntax (^foo^)
        $this->value = preg_replace('/"(.*?)"/', '^\1^', $this->value);
      }
    }

    parent::_cql();
  }

  /**
   * Provide a simple textfield for equality.
   */
  public function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => $this->definition['title'],
      '#default_value' => $this->value,
    );
  }
}
