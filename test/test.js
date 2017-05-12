var assert = require('assert')
  , os = require('os')
  , expect = require('chai').expect
  , mod = require('../')
  // These will have results returned by mod.list(), used throughout the suite:
  , refList
  , refList2

if (process.platform !== 'win32') {
  console.error('This module is only meant for Windows platforms.\n' +
    'Aborting tests.\n');
  return
}

describe('windows-users module', function() {
  it('should export functions: list, getDetails, getGlobalGroups, getLocalGroups',
  function() {
    expect(mod.list).to.be.a('function')
    expect(mod.getDetails).to.be.a('function')
    expect(mod.getGlobalGroups).to.be.a('function')
    expect(mod.getLocalGroups).to.be.a('function')
  })

  var constantNames = [
    'NORMAL', 'TEMP_DUPLICATE', 'INTERDOMAIN_TRUST', 'WORKSTATION_TRUST',
    'SERVER_TRUST'
  ]

  it('should export these distinct numeric constants: ' + constantNames.join(', '),
  function() {
    var i, ids = constantNames

    expect(mod.constants).to.be.an('object')

    for (i in ids)
      expect(mod.constants).to.have.property(ids[i]).that.is.a('number')

    for (i in ids) { // Each value is distinct from the others
      for (var j = i + 1; j < ids.length; j++)
        expect(mod.constants[ids[i]]).to.not.equal(mod.constants[ids[j]])
    }

    for (i in ids)   // Prove that each is constant
      expect(function(){ mod.constants[ids[i]]++ })
        .to.not.change(mod.constants, ids[i])
  })

  var emptyArgs   = [ undefined, null, '', new String() ]
    , invalidNameArgs = [ 42, true, [], {} ]
    , listInvalid1stArg = [ 42, true, [], new Date() ]
    , badFilterOptType = [ 'dummy', true, null, undefined, [], {} ]
    , badDetailedOpt = [ 'dummy', null, undefined, 1, [], {} ]
    , badHostName = "IWILLNOTCALLMYCOMPUTERANINAPPROPRIATENAME"
    , badUserName = "_                  _"

    , re_errNoUsername = /Must provide username/
    , re_errEmptyUsername = /Username cannot be empty/
    , re_errUsernameType = /Invalid type for username argument/
    , re_errArg1Type = /Invalid type for first argument/
    , re_errArg2Type = /Invalid type for second argument/
    , re_errArg3Type = /Invalid type for third argument/
    , re_errNoCb = /Must provide callback when giving a hostname/
    , re_errDetailedType = /Invalid type for option 'detailed'/
    , re_errFilterType = /Only numeric values allowed for option 'filter'/
    , re_errFilterRange = /Out-of-range value for option 'filter'/
    , re_errNotFound = /(could not be found)|(does not exist)/
    , re_errHostnameAccess =
       /(access is denied)|(hostname is unrecognized)|(no RPC server available)/

  function dummyFunc(err, data) {
    assert(false, 'This dummy function should never get called!')
  }

  var shortInfoItem = {
    name: { type: 'string' },
    fullName: { type: 'string', alt: '' },
    comment: { type: 'string', alt: '' },
    accountType: { type: 'string' },
    disabled: { type: 'boolean' },
    lockedOut: { type: 'boolean' },
    passwdRequired: { type: 'boolean' },
    passwdCanChange: { type: 'boolean' },
    passwdExpired: { type: 'boolean' },
    passwdNeverExpires: { type: 'boolean' },
    encryptedPasswdOK: { type: 'boolean' },
    smartcardRequired: { type: 'boolean' },
    useOnlyDES: { type: 'boolean' },
    noPreauthRequired: { type: 'boolean' },
    notDelegated: { type: 'boolean' },
    trustedForDeleg: { type: 'boolean' },
    trustedToAuthDeleg: { type: 'boolean' }
  }
  var fullDetailsItem = {
    sid: { type: 'string' },
    primaryGroupId: { type: 'number', alt: null },
    homeDir: { type: 'string', alt: '' },
    homeDirDrive: { type: 'string', alt: '' },
    profilePath: { type: 'string', alt: '' },
    scriptPath: { type: 'string', alt: '' },
    maxStorage: { type: 'number', alt: null },
    privilegeLevel: { type: 'string' },
    isAccountsOperator: { type: 'boolean' },
    isServerOperator: { type: 'boolean' },
    isPrintOperator: { type: 'boolean' },
    userComment: { type: 'string', alt: '' },
    countryCode: { type: 'number' },
    codePage: { type: 'number' },
    appParams: { type: 'string', alt: '' },
    passwdAge: { type: 'number' },
    accountExpires: { type: 'number', alt: null },
    logonHours: { type: 'array', alt: null },
    workstations: { type: 'string', alt: '' },
    logonServer: { type: 'string', alt: '' },
    lastLogon: { type: 'number' },
    logonCount: { type: 'number', alt: null },
    badPasswdCount: { type: 'number', alt: null }
  }

  function timeDesensitize(data)
  {
    data.passwdExpired = false
    if ('passwdAge' in data) data.passwdAge = 0
  }

  function validateLogonHours(arr)
  {
    expect(arr).to.be.an('array').that.has.lengthOf(7)
    for (var day = 0; day < 7; day++) {
      var hrList = arr[day]
      var tempMap = {}
      expect(hrList).to.be.an('array').with.lengthOf.at.most(24)

      for (var h = 0; h < hrList.length; h++) {
        expect(hrList[h]).to.be.a('number').that.is.at.least(0).and.at.most(23)
        // Test for duplicates
        expect(tempMap[hrList[h]]).to.be.undefined
        tempMap[hrList[h]] = true
      }
    }
  }

  function validateUserInfo(info, isFull)
  {
    assert(info && typeof info === 'object')

    // Ensure info contains all the properties spec'd in lookup 'shortInfoItem'
    expect(info).to.include.all.keys(Object.keys(shortInfoItem))

    var prop, lookup

    if (isFull) {
      // Ensure info also contains all properties spec'd in lookup 'fullDetailsItem'
      expect(info).to.include.all.keys(Object.keys(fullDetailsItem))

      for (prop in info) {
        if (!(prop in shortInfoItem)) {
          lookup = fullDetailsItem
          expect(lookup).to.have.property(prop)

          if (prop === 'logonHours' && info.logonHours)
            validateLogonHours(info.logonHours)
        }
        else lookup = shortInfoItem

        if (lookup[prop].alt === undefined)
          expect(info[prop]).to.be.a(lookup[prop].type)
        else if (typeof info[prop] !== lookup[prop].type)
          expect(info[prop]).to.equal(lookup[prop].alt)
      }
    }
    else {
      // Ensure all properties in given info are valid
      for (prop in info) {
        expect(shortInfoItem).to.have.property(prop)
        if (shortInfoItem[prop].alt === undefined)
          expect(info[prop]).to.be.a(shortInfoItem[prop].type)
        else if (typeof info[prop] !== shortInfoItem[prop].type)
          expect(info[prop]).to.equal(shortInfoItem[prop].alt)
      }
    }
  }

  describe('list() synchronous call', function() {

    // Here the reference data is collected, as a side effect of testing the
    // basic form of the function:
    before('should return an array when no arguments are given', function() {
      refList = mod.list()
      expect(refList).to.be.instanceof(Array)
      if (refList.length == 0) {
        console.warn(
          'NO USERS DEFINED ON THIS SYSTEM!\n' +
          'NO MEANINGFUL TESTS CAN BE DONE, SO TEST SUITE WILL BE ABORTED.\n' +
          'SORRY!'
        );
        process.exit()
      }
    })

    it('results for no-argument call should contain only distinct strings',
    function() {

      for (var i = 0; i < refList.length; i++) {
        expect(refList[i]).to.be.a('string').that.is.not.empty

        for (var j = i + 1; j < refList.length; j++)
          expect(refList[i]).to.not.equal(refList[j])
      }
    })

    it('results should be the same as for no-arg call when passed a single' +
       ' empty arg (undefined, null, empty string)', function() {

      emptyArgs.forEach(function(el) {
        expect(refList).to.have.members(mod.list(el))
      })
    })

    it('should throw an exception when 1st argument has invalid type', function() {

      for (var i = 0; i < listInvalid1stArg.length; i++) {
        expect(function(){ mod.list(listInvalid1stArg[i]) })
          .to.throw(Error, re_errArg1Type)
      }
    })

    it('should throw an exception when non-empty servername arg is given' +
       ' without passing a callback', function() {

      // Name doesn't matter - if given without callback, it is rejected
      expect(function(){ mod.list('SOMESERVER') })
        .to.throw(Error, re_errNoCb)
    })

    it('results should be the same as for no-arg call when passed an empty object',
    function() {
      expect(refList).to.have.members(mod.list({}))
    })

    it('results should be the same as for no-arg call when passed an object ' +
       'with only an unrecognized property', function() {

      expect(refList).to.have.members(mod.list({ unknownProp: 999 }))
    })

    it('should throw an exception when options object is given with invalid ' +
       'value for "filter" property', function() {

      for (var i = 0; i < badFilterOptType.length; i++) {
        expect(function(){ mod.list({ filter: badFilterOptType[i] }) })
          .to.throw(Error, re_errFilterType)
      }

      expect(function(){ mod.list({ filter: -1 }) })
        .to.throw(Error, re_errFilterRange)
      expect(function(){ mod.list({ filter: Math.floor(Math.pow(2,32)) }) })
        .to.throw(Error, re_errFilterRange)
    })

    it('should throw an exception when options object is given with invalid ' +
       'value for "detailed" property', function() {

      for (var i = 0; i < badDetailedOpt.length; i++) {
        expect(function(){ mod.list({ detailed: badDetailedOpt[i] }) })
          .to.throw(Error, re_errDetailedType)
      }
    })

    it('results should be the same as for no-arg call when only ' +
       ' detailed:false is passed in an object argument', function() {

      expect(refList).to.have.members(mod.list({ detailed: false }))
    })

    it('should return an array that contains only names seen from no-arg call ' +
       'when a valid filter constant is passed in an object', function() {

      for (var val in mod.constants) {
        expect(refList).to.include.members(mod.list({ filter: mod.constants[val] }))
      }
    })

    it('should return a list of objects with specific fields for every user ' +
       'when detailed:true is passed in an object argument', function() {

      // Side effect: cache the 2nd reference list
      refList2 = mod.list({ detailed: true })

      for (var i = 0; i < refList2.length; i++) {
        expect(refList).to.contain(refList2[i].name)
        validateUserInfo(refList2[i])
        timeDesensitize(refList2[i])
      }
    })
  })

  describe('list() asynchronous call', function() {

    it('should pass back already-seen array of names when no other arguments ' +
       'are given', function(done) {

      mod.list(function(err, data) {
        expect(err).to.be.null
        expect(refList).to.have.members(data)
        done()
      })
    })

    it('results should be the same as for no-options call when passed an ' +
       ' empty value for 1st arg (undefined, null, empty string)', function(done) {

      // The idea here is that the empty arg can stand in place of a hostname,
      // which is OK because it implies the localhost. It is never OK for it to
      // stand in place of the options object.

      function testNextEl(n) {
        if (emptyArgs.length <= n) return done()

        mod.list(emptyArgs[n], function(err, data) {
          expect(err).to.be.null
          expect(refList).to.have.members(data)

          testNextEl(n + 1)
        })
      }

      testNextEl(0)
    })

    it('results should be the same as for no-options call when passed an empty object',
    function(done) {

      mod.list({}, function(err, data) {
        expect(err).to.be.null
        expect(refList).to.have.members(data)
        done()
      })
    })

    it('results should be the same as for no-options call when passed an object ' +
       'with only an unrecognized property', function(done) {

      mod.list({ unknownProp: 999 }, function(err, data) {
        expect(err).to.be.null
        expect(refList).to.have.members(data)
        done()
      })
    })

    it('results should be the same as for no-options call when only ' +
       ' detailed:false is passed in an object argument', function(done) {

      mod.list({ detailed: false }, function(err, data) {
        expect(err).to.be.null
        expect(refList).to.have.members(data)
        done()
      })
    })

    it('should return an array that contains only names seen from no-options ' +
       'call when a valid filter constant is passed in an object', function(done) {

      var constNames = Object.keys(mod.constants)

      function testNextConstant(n) {
        if (constNames.length <= n) return done()

        mod.list({ filter: mod.constants[constNames[n]] }, function(err, data) {
          expect(err).to.be.null
          expect(refList).to.include.members(data)

          testNextConstant(n + 1)
        })
      }

      testNextConstant(0)
    })

    it('should return already-seen list of objects when detailed:true is ' +
       'passed in an object argument', function(done) {

      mod.list({ detailed: true }, function(err, data) {
        expect(err).to.be.null
        expect(data).to.be.an('array')

        // Neutralize the time-sensitive fields, for comparison convenience:
        for (var i = 0; i < data.length; i++) timeDesensitize(data[i])

        expect(refList2).to.deep.have.members(data)
        done()
      })
    })

    it('should throw an exception when passed invalid type for hostname',
    function() {
      for (var i = 0; i < invalidNameArgs.length; i++) {
        var badArg = invalidNameArgs[i]
        expect(function(){ mod.list({detailed:false}, badArg, dummyFunc) })
          .to.throw(Error, re_errArg2Type)
      }
    })

    it('should pass back an error when given an unrecognized hostname',
    function(done) {
      mod.list({detailed:false}, badHostName, function(err, data) {
        expect(err).to.be.an('error')
        //expect(err.message).to.match(re_errHostnameAccess)
// TODO: Get the correct match for the above on WinXP
        done()
      })
    })

    it('should pass back valid data when given hostname of localhost',
    function(done) {
      mod.list(os.hostname(), function(err, data1) {
        expect(err).to.be.null
        expect(refList).to.have.members(data1)

        mod.list({detailed:false}, os.hostname(), function(err, data2) {
          expect(err).to.be.null
          expect(data1).to.have.members(data2)

          mod.list({detailed:true}, os.hostname(), function(err, data3) {
            expect(err).to.be.null

            // Neutralize the time-sensitive fields, for comparison convenience:
            for (var i = 0; i < data3.length; i++) timeDesensitize(data3[i])

            expect(refList2).to.deep.have.members(data3)
            done()
          })
        })
      })
    })

  })

  describe('getDetails() synchronous call', function() {

    it('should throw an exception when given no arguments', function() {

      expect(function(){ mod.getDetails() }).to.throw(Error, re_errNoUsername)
    })

    it('should throw an exception when 1st argument is not a string', function() {

      for (var i = 0; i < listInvalid1stArg.length; i++) {
        expect(function(){ mod.getDetails(listInvalid1stArg[i]) })
          .to.throw(Error, re_errUsernameType)
      }
    })

    it('should throw an exception when 1st arg is an empty string', function() {

      expect(function(){ mod.getDetails('') })
        .to.throw(Error, re_errEmptyUsername)

      expect(function(){ mod.getDetails(new String('')) })
        .to.throw(Error, re_errEmptyUsername)
    })

    it('should return an object matching the one with same username among ' +
       'those fetched by list({detailed:true}) when passed that name',
    function() {
      var data = mod.getDetails(refList2[0].name)
      timeDesensitize(data)
      expect(data).to.eql(refList2[0])
    })

    it('results should be the same data as for one-arg call when given ' +
       'false as 2nd arg', function() {

      var data = mod.getDetails(refList2[0].name, false)
      timeDesensitize(data)
      expect(data).to.eql(refList2[0])
    })

    it('results should include the same data as for one-arg call, plus ' +
       'several other specific fields, when given true as 2nd arg', function() {

      var info = mod.getDetails(refList2[0].name, true)
      expect(info).to.not.eql(refList2[0])
      expect(info).to.contain.all.keys(refList2[0])
      validateUserInfo(info, true)
    })

    it('results should be the same as for no-hostname call when the hostname ' +
       'arg is empty (undefined, null, empty string)', function() {

      var username = refList2[0].name
      var fullInfo = mod.getDetails(username, true)
      timeDesensitize(fullInfo)

      emptyArgs.forEach(function(el) {
        var info = mod.getDetails(username, false, el)
        timeDesensitize(info)
        expect(info).to.eql(refList2[0])

        var deepInfo = mod.getDetails(username, true, el)
        timeDesensitize(deepInfo)
        expect(deepInfo).to.eql(fullInfo)
      })
    })

    it('should throw an exception when non-empty servername arg is given' +
       ' without passing a callback', function() {

      expect(function(){ mod.getDetails(refList[0], 'WHATEVER') })
        .to.throw(Error, re_errNoCb)
    })
  })

  describe('getDetails() asynchronous call', function() {

    it('should pass to the callback an object matching the one with same ' +
       'username among those already fetched by list({detailed:true}) when ' +
       'passed that name', function(done) {

      mod.getDetails(refList2[0].name, function(err, data) {
        expect(err).to.be.null
        timeDesensitize(data)
        expect(data).to.eql(refList2[0])
        done()
      })
    })

    it('should pass to the callback the same data as for one-arg call when ' +
       'given false as 2nd arg', function(done) {

      mod.getDetails(refList2[0].name, false, function(err, data) {
        expect(err).to.be.null
        timeDesensitize(data)
        expect(data).to.eql(refList2[0])
        done()
      })
    })

    it('should pass to the callback the same data as for one-arg call, plus ' +
       'other specific fields, when given true as 2nd arg', function(done) {

      mod.getDetails(refList2[0].name, true, function(err, data) {
        expect(err).to.be.null
        expect(data).to.not.eql(refList2[0])
        expect(data).to.contain.all.keys(refList2[0])
        validateUserInfo(data, true)
        done()
      })
    })

    it('should pass to the callback the same data returned for synchronous ' +
       'call when the hostname arg is empty (undefined, null, empty string)',
    function(done) {
      var username = refList2[0].name
      var fullInfo = mod.getDetails(username, true)
      timeDesensitize(fullInfo)

      function nextTest(n, full) {
        if (n >= emptyArgs.length) {
          if (full) return done()
          return nextTest(0, true)
        }

        mod.getDetails(username, full, emptyArgs[n], function(err, data) {
          expect(err).to.be.null
          timeDesensitize(data)
          expect(data).to.eql(full ? fullInfo : refList2[0])
          return nextTest(n + 1, full)
        })
      }

      nextTest(0, false)
    })

    it('should pass back an error when given an unrecognized username',
    function(done) {
      mod.getDetails(badUserName, function(err, data) {
        expect(err).to.be.an('error')
        expect(err.message).to.match(re_errNotFound)
        done()
      })
    })

    it('should throw an exception when passed invalid type for hostname',
    function() {
      // In this arg context, boolean is valid in the 2nd position, so we remove
      // ambiguity by providing the optional 'fullDetails' flag, forcing the
      // invalid boolean arg (true) to the 3rd position, so that when badArg is
      // seen, the argsResolver knows it's meant to be as the hostname.
      for (var i = 0; i < invalidNameArgs.length; i++) {
        var badArg = invalidNameArgs[i]
        expect(function(){ mod.getDetails(refList[0], false, badArg, dummyFunc) })
          .to.throw(Error, re_errArg3Type)
      }
    })

    it('should pass back an error when given an unrecognized hostname',
    function(done) {
      mod.getDetails(refList[0], badHostName, function(err, data) {
        expect(err).to.be.an('error')
        //expect(err.message).to.match(re_errHostnameAccess)
        done()
      })
    })

    it('should pass back valid data when given hostname of localhost',
    function(done) {
      var username = refList2[0].name
      var fullInfo = mod.getDetails(username, true)
      timeDesensitize(fullInfo)

      mod.getDetails(username, os.hostname(), function(err, data1) {
        expect(err).to.be.null
        timeDesensitize(data1)
        expect(refList2[0]).to.eql(data1)

        mod.getDetails(username, false, os.hostname(), function(err, data2) {
          expect(err).to.be.null
          timeDesensitize(data2)
          expect(data1).to.eql(data2)

          mod.getDetails(username, true, os.hostname(), function(err, data3) {
            expect(err).to.be.null
            timeDesensitize(data3)
            expect(data3).to.eql(fullInfo)
            done()
          })
        })
      })
    })

  })

  var localGroups0; // for caching reference data

  describe('getLocalGroups() synchronous call', function() {

    it('should throw an exception when given no arguments', function() {

      expect(function(){ mod.getLocalGroups() }).to.throw(Error, re_errNoUsername)
    })

    it('should throw an exception when 1st argument is not a non-empty string',
    function() {
      for (var i = 0; i < listInvalid1stArg.length; i++)
        expect(function(){ mod.getLocalGroups(listInvalid1stArg[i]) })
          .to.throw(Error, re_errUsernameType)

      expect(function(){ mod.getLocalGroups('') })
        .to.throw(Error, re_errEmptyUsername)

      expect(function(){ mod.getLocalGroups(new String('')) })
        .to.throw(Error, re_errEmptyUsername)
    })

    it('should return an array of only unique and distinct strings when ' +
       'passed a valid username', function() {

      localGroups0 = mod.getLocalGroups(refList[0])
      expect(localGroups0).to.be.an('array')

      for (var i = 0; i < localGroups0.length; i++) {
        expect(localGroups0[i]).to.be.a('string').that.is.not.empty

        for (var j = i + 1; j < localGroups0.length; j++)
          expect(localGroups0[i]).to.not.equal(localGroups0[j])
      }
    })
  })

  describe('getLocalGroups() asynchronous call', function() {

    it('should pass to the callback the same data returned for synchronous ' +
       'call when no hostname is given', function(done) {

      mod.getLocalGroups(refList[0], function(err, data) {
        expect(err).to.be.null
        expect(data).to.have.members(localGroups0)
        done()
      })
    })

    it('should pass to the callback the same data returned for synchronous ' +
       'call when the hostname arg is empty (undefined, null, empty string)',
    function(done) {
      var username = refList[0]

      function nextTest(n) {
        if (n >= emptyArgs.length) return done()

        mod.getLocalGroups(username, emptyArgs[n], function(err, data) {
          expect(err).to.be.null
          expect(data).to.have.members(localGroups0)
          return nextTest(n + 1)
        })
      }

      nextTest(0)
    })

    it('should pass back an error when given an unrecognized username',
    function(done) {
      mod.getLocalGroups(badUserName, function(err, data) {
        expect(err).to.be.an('error')
        expect(err.message).to.match(re_errNotFound)
        done()
      })
    })


    it('should throw an exception when passed invalid type for hostname',
    function() {
      for (var i = 0; i < invalidNameArgs.length; i++) {
        var badArg = invalidNameArgs[i]
        expect(function(){ mod.getLocalGroups(refList[0], badArg, dummyFunc) })
          .to.throw(Error, re_errArg2Type)
      }
    })

    it('should pass back an error when given an unrecognized hostname',
    function(done) {
      mod.getLocalGroups(refList[0], badHostName, function(err, data) {
        expect(err).to.be.an('error')
        //expect(err.message).to.match(re_errHostnameAccess)
        done()
      })
    })

    it('should pass back valid data when given hostname of localhost',
    function(done) {
      mod.getLocalGroups(refList[0], os.hostname(), function(err, data) {
        expect(err).to.be.null
        expect(data).to.have.members(localGroups0)
        done()
      })
    })

  })

  var globalGroups0; // for caching reference data

  describe('getGlobalGroups() synchronous call', function() {

    it('should throw an exception when given no arguments', function() {

      expect(function(){ mod.getGlobalGroups() }).to.throw(Error, re_errNoUsername)
    })

    it('should throw an exception when 1st argument is not a non-empty string',
    function() {
      for (var i = 0; i < listInvalid1stArg.length; i++)
        expect(function(){ mod.getGlobalGroups(listInvalid1stArg[i]) })
          .to.throw(Error, re_errUsernameType)

      expect(function(){ mod.getGlobalGroups('') })
        .to.throw(Error, re_errEmptyUsername)

      expect(function(){ mod.getGlobalGroups(new String('')) })
        .to.throw(Error, re_errEmptyUsername)
    })

    it('should return an array of only unique and distinct strings when ' +
       'passed a valid username', function() {

      globalGroups0 = mod.getGlobalGroups(refList[0])
      expect(globalGroups0).to.be.an('array')

      for (var i = 0; i < globalGroups0.length; i++) {
        expect(globalGroups0[i]).to.be.a('string').that.is.not.empty

        for (var j = i + 1; j < globalGroups0.length; j++)
          expect(globalGroups0[i]).to.not.equal(globalGroups0[j])
      }
    })
  })

  describe('getGlobalGroups() asynchronous call', function() {

    it('should pass to the callback the same data returned for synchronous ' +
       'call when no hostname is given', function(done) {

      mod.getGlobalGroups(refList[0], function(err, data) {
        expect(err).to.be.null
        expect(data).to.have.members(globalGroups0)
        done()
      })
    })

    it('should pass to the callback the same data returned for synchronous ' +
       'call when the hostname arg is empty (undefined, null, empty string)',
    function(done) {
      var username = refList[0]

      function nextTest(n) {
        if (n >= emptyArgs.length) return done()

        mod.getGlobalGroups(username, emptyArgs[n], function(err, data) {
          expect(err).to.be.null
          expect(data).to.have.members(globalGroups0)
          return nextTest(n + 1)
        })
      }

      nextTest(0)
    })

    it('should pass back an error when given an unrecognized username',
    function(done) {
      mod.getGlobalGroups(badUserName, function(err, data) {
        expect(err).to.be.an('error')
        expect(err.message).to.match(re_errNotFound)
        done()
      })
    })


    it('should throw an exception when passed invalid type for hostname',
    function() {
      for (var i = 0; i < invalidNameArgs.length; i++) {
        var badArg = invalidNameArgs[i]
        expect(function(){ mod.getGlobalGroups(refList[0], badArg, dummyFunc) })
          .to.throw(Error, re_errArg2Type)
      }
    })

    it('should pass back an error when given an unrecognized hostname',
    function(done) {
      mod.getGlobalGroups(refList[0], badHostName, function(err, data) {
        expect(err).to.be.an('error')
        //expect(err.message).to.match(re_errHostnameAccess)
        done()
      })
    })

    it('should pass back valid data when given hostname of localhost',
    function(done) {
      mod.getGlobalGroups(refList[0], os.hostname(), function(err, data) {
        expect(err).to.be.null
        expect(data).to.have.members(globalGroups0)
        done()
      })
    })

  })

})

