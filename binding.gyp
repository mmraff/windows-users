{
  "targets": [
    {
      "target_name": "winusers",
      "sources": [
        "src/argsresolv.cc",
        "src/usererrs.cc",
        "src/wstrutils.cc",
        "src/userinfo.cc",
        "src/xferlist.cc",
        "src/api2info.cc",
        "src/wraputils.cc",
        "src/userenum.cc",
        "src/enumwrap.cc",
        "src/deepinfo.cc",
        "src/ugetinfo.cc",
        "src/infowrap.cc",
        "src/usergrps.cc",
        "src/grpswrap.cc",
        "src/constants.cc",
        "src/module.cc"
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'AdditionalOptions': [ '/EHsc'] 
        }
      }
    }
  ]
}
