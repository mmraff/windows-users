var addon = require('bindings')('winusers')

// addon.path might prove useful, but we don't want to export it
for (var prop in addon) {
  if ('function' === typeof addon[prop])
    module.exports[prop] = addon[prop]
}
if (addon.constants)
  module.exports.constants = addon.constants

