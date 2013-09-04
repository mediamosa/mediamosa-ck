<?php
/**
 * @file
 * Views filter metadata.
 */

class mediamosa_ck_views_filter_metadata extends mediamosa_ck_views_filter {

  /**
   * Returns options.
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['metadata_type'] = array('default' => 'title;CHAR');
    return $options;
  }

  /**
   * Return a form array for the options.
   */
  public function options_form(&$form, &$form_state) {
    // Get all metadata fields.
    $options = array('titledesc;CHAR' => 'title,description,filename');
    $types = mediamosa_ck::get_metadata_fields();
    foreach ($types as $type) {
      $options[$type['title']] = array();
      if (!empty($type['fields']) && is_array($type['fields'])) {
        foreach ($type['fields'] as $key => $value) {
          $options[$type['title']][$key . ';' . $value] = $key;
        }
      }
    }
    $form['metadata_type'] = array(
      '#type' => 'select',
      '#title' => t('Type'),
      '#options' => $options,
      '#default_value' => isset($this->options['metadata_type']) ? $this->options['metadata_type'] : 'title;CHAR',
    );

    parent::options_form($form, $form_state);
  }

  /**
   * Add input to filter data.
   */
  public function query() {
    if (!empty($this->value)) {
      foreach ($this->value as $value) {
        if (!empty($value['metadata'])) {
          $value['metadata'] = check_plain($value['metadata']);
          $cql_or = array();
          $metadata_type = isset($this->options['metadata_type']) ? $this->options['metadata_type'] : 'title';
          list($metadata_name, $metadata_format) = explode(';', $metadata_type);
          if ($metadata_name == 'titledesc') {
            $cql_or[] = 'title="' . $value['metadata'] . '"';
            $cql_or[] = 'description="' . $value['metadata'] . '"';
            $cql_or[] = 'filename="' . $value['metadata'] . '"';
          }
          else {
            $cql_or[] = $metadata_name . '="' . $value['metadata'] . '"';
          }
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

    $metadata_type = isset($this->options['metadata_type']) ? $this->options['metadata_type'] : 'title;CHAR';
    list($metadata_name, $metadata_format) = explode(';', $metadata_type);

    switch ($metadata_format) {
      case 'CHAR':
        $type = 'textfield';
        if ($metadata_name == 'tag' && module_exists('taxonomy') && module_exists('mediamosa_ck_taxonomy')) {
          $extra_form = array(
            '#autocomplete_path' => 'mediamosa/ck/taxonomy/autocomplete',
            '#maxlength' => 1024,
          );
        }
        break;

      case 'DATETIME':
        $type = 'date';
        break;

      default:
        $type = 'textfield';
        break;
    }

    $form['value']['metadata'] = array(
      '#type' => $type,
      '#default_value' => '',
    );

    if (!empty($extra_form)) {
      $form['value']['metadata'] += $extra_form;
    }
  }
}
