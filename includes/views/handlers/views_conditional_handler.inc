<?php

/**
 * @file
 * Handles conditionals in Views.  IF (xxx) THEN (yyy)...
 */

class views_conditional_handler extends views_handler_field {
  // Conditional oeprators
  public $conditions = array(
    1 => t('Equal to'),
    2 => t('NOT equal to'),
    3 => t('Greater than'),
    4 => t('Less than'),
    5 => t('Empty'),
    6 => t('NOT empty'),
  );

  /**
   * Set default values for form elements.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['label']['default'] = NULL;

    $options['if'] = array('default' => '');
    $options['condition'] = array('default' => '');
    $options['equalto'] = array('default' => '');
    $options['then'] = array('default' => '');
    $options['or'] = array('default' => '');

    return $options;
  }

  /**
   * Views form elements.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['relationship']['#access'] = FALSE;

    // Display all labels weighted less than the current label.
    $fields = array(
      0 => '- ' . t('no field selected') . ' -',
    ) + $this->view->display_handler->get_field_labels();

    // We only use fields up to (not including) this one.
    unset($fields[$this->options['id']]);

    $form['if'] = array(
      '#type' => 'select',
      '#title' => t('If this field...'),
      '#options' => $fields,
      '#default_value' => $this->options['if'],
    );

    $form['condition'] = array(
      '#type' => 'select',
      '#title' => t('Is...'),
      '#options' => $this->conditions,
      '#default_value' => $this->options['condition']
    );

    $form['equalto'] = array(
      '#type' => 'textfield',
      '#title' => t('This value'),
      '#description' => t('Input a value to compare the field against.  Replacement variables may be used'),
      '#default_value' => $this->options['equalto'],
    );

    $form['then'] = array(
      '#type' => 'textfield',
      '#title' => t('Then output this...'),
      '#description' => t('Input what should be output.  Replacement variables may be used.'),
      '#default_value' => $this->options['then'],
    );

    $form['or'] = array(
      '#type' => 'textfield',
      '#title' => t('Otherwise, output this...'),
      '#description' => t('Input what should be output if the above conditions do NOT evaluate to true.'),
      '#default_value' => $this->options['or']
    );

    $form['replacements'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('Replacement Variables')
    );

    $form['replacements']['notice'] = array(
      '#markup' => 'You may use any of these replacement variables in the "equals" or the "output" text fields.  If you wish to use brackets ([ or ]), replace them with %5D or %5E.',
      '#prefix' => '<p>',
      '#suffix' => '</p>'
    );

    $items = array(
      'DATE_UNIX => Current date / time, in UNIX timestamp format (' . REQUEST_TIME . ')',
      'DATE_STAMP => Current date / time, in standard format (' . format_date(REQUEST_TIME) . ')', 
    );
    foreach ($this->view->display_handler->get_field_labels() AS $item => $title) {
      $items[] = "[$item] => $title";
    }

    $form['replacements']['variables'] = array(
      '#markup' => theme('item_list', array('items' => $items))
    );
  }

  /**
   * Check that everything is in order.
   */
  function options_validate($form, &$form_state) {
    $values = &$form_state['values']['options'];
    if (empty($values['if']) || empty($values['condition']) || empty($values['equalto'])) {
      if (empty($values['if'])) {
        form_set_error('if', t("Please specify a valid field to run a condition on."));
      }
      if (empty($values['condition'])) {
        form_set_error('condition', t("Please select a conditional operator."));
      }
      if (empty($values['equalto']) && !in_array($values['condition'], array(5, 6))) {
        form_set_error('condition', t("Please specify something to compare with."));
      }
    }
  }

  /**
   * Defines summary text for Fields category in Views.
   */
  function admin_summary() {
    if (!empty($this->options['if']) && !empty($this->options['condition']) && (!empty($this->options['equalto']) || in_array($this->options['condition'], array(5, 6))) && trim($this->options['then']) != '') {
      return t('If !if is !condition !equalto, output !then, else output !or', array(
        '!if' => $this->options['if'],
        '!condition' => $this->conditions[$this->options['condition']],
        '!equalto' => $this->options['equalto'],
        '!then' => $this->options['then'],
        '!or' => $this->options['or']
      ));
    }
    else {
      return t('Invalid field selection');
    }
  }

  /**
   * Remove advanced rendering options from form.
   */
  function allow_advanced_render() {
    return FALSE;
  }

  /**
   * Defines field alias.  No query manipulation necessary.
   */
  function query() {
    $this->field_alias = 'views_conditional_' . $this->position;
  }

  function clean_var($var, $key) {
    $f = array();
    if (isset($var->view->query->pager->display->handler->options['fields'][$key])) {
      $f = $var->view->query->pager->display->handler->options['fields'][$key];
    }

    $unparsed = trim($var->last_render);
    $parse = true;
    if (!empty($f['element_type'])) {
      $parse = false;
    }

    if (isset($f['alter']['make_link']) && $f['alter']['make_link'] == 1) {
      $parse = false;
    }

    if (isset($f['alter']['prefix']) || isset($f['alter']['suffix'])) {
      $parse = false;
    }

    if ($parse) {
      $unparsed = strip_tags($unparsed);
    }

    return trim($unparsed);
  }

  /**
   * Renders the final output based on conditional input.
   */
  function render($values) {
    $if = $this->options['if'];
    $condition = $this->options['condition'];
    $equalto = $this->options['equalto'];
    $then = $this->options['then'];
    $or = ($this->options['or'] ? $this->options['or'] : '');

    // Gather field information.
    $fields = $this->view->display_handler->get_handlers('field');

    // Search through field information for possible replacement variables.
    foreach ($this->view->display_handler->get_field_labels() AS $key => $var) {
      // If we find a replacement variable, replace it.
      if (strpos($then, "[$key]") !== false) {
        $field = $this->clean_var($fields[$key], $key);
        $then = str_replace("[$key]", $field, $then);
      }
      else if (strpos($or, "[$key]") !== false) {
        $field = $this->clean_var($fields[$key], $key);
        $or = str_replace("[$key]", $field, $or);
      }
    }

    // If we find a date stamp replacement, replace that.
    if (strpos($then, 'DATE_STAMP') !== false) {
      $then = str_replace('DATE_STAMP', format_date(REQUEST_TIME), $then);
    }
    else if (strpos($then, 'DATE_STAMP') !== false) {
      $or = str_replace('DATE_STAMP', format_date(REQUEST_TIME), $or);
    }

    // If we find a unix date stamp replacement, replace that.
    if (strpos($then, 'DATE_UNIX') !== false) {
      $then = str_replace('DATE_UNIX', REQUEST_TIME, $then);
    }
    else if (strpos($or, 'DATE_UNIX') !== false) {
      $or = str_replace('DATE_UNIX', REQUEST_TIME, $or);
    }

    // Strip tags on the "if" field.  Otherwise it appears to output as <div class="xxx">Field data</div>...
    // ...which of course makes it dificult to compare.
    $r = trim(strip_tags($fields["$if"]->last_render));

    // Run conditions.
    switch ($condition) {
      // Equal to
      case 1:
        if ($r == $equalto) {
          return $then;
        }
        else {
          return $or;
        }
      break;
      // Not equal to
      case 2:
        if ($r !== $equalto) {
          return $then;
        }
        else {
          return $or;
        }
      break;
      // Greater than
      case 3:
        if ($r > $equalto) {
          return $then;
        }
        else {
          return $or;
        }
      break;
      // Less than
      case 4:
        if ($r < $equalto) {
          return $then;
        }
        else {
          return $or;
        }
      break;
      // Empty
      case 5:
        if (empty($r)) {
          return $then;
        }
        else {
          return $or;
        }
      break;
      // Not empty
      case 6:
        if (!empty($r)) {
          return $then;
        }
        else {
          return $or;
        }
      break;
    }
  }

}
