# windows-users
A native addon for Node.js to query user accounts on Windows platforms


## Install
<pre style="color:#ccc;background-color:#222;">
C:\Users\myUser><b>npm install windows-users</b>
</pre>
**Note to users of pre-C++-2011 compilers only:** During the build, the compiler will print many
warnings like this:
<pre style="color:#ff0;background-color:#222;">
..\src\deepinfo.cc(63): warning C4482: nonstandard extension used: enum 'UserField' used in qualified name
</pre>
These can be safely ignored.

## Usage
```js
var users = require('windows-users')

var nameList = users.list()
console.log('The user accounts on this system are:', nameList.join(', '))
nameList.forEach(function(username) {
  var account = users.getDetails(username)
  if (account.accountType === 'administrator' && !account.disabled) {
    // ...
  }
})

var infoList = users.list({detailed: true})
for (var i = 0; i < infoList.length; i++) {
  var account = infoList[i]
  if (account.disabled || account.lockedOut) continue
  var fullInfo = users.getDetails(account.name)
  console.log('Account %s has SID: %s', fullInfo.name, fullInfo.sid)
  if (fullInfo.lastLogon > 0)
    console.log('Last logon was', new Date(fullInfo.lastLogon * 1000))
  var localGrps = users.getLocalGroups(account.name)
  var globalGrps = users.getGlobalGroups(account.name)
  // ...
}
```
For each of the four available functions, the user may expect the result of
querying the localhost as the synchronous return value *if no callback function
is added to the argument list*. Simply supply a callback function as the last
argument to turn the call asynchronous.  
A callback function is *required* if a host other than the localhost is to be
queried.  

**Note:** In some environments, the synchronous functions may throw an `Error` of
`"access is denied to current user"`, and any of the asynchronous functions may
pass this error to the callback. This would be a consequence of the user's
permissions versus the particular security configuration of the domain.

---
## API

<a name="list-sync"></a>

### users.list([options])
*Synchronous.* Returns a list of all user accounts known to the local system,
optionally according to the specified options.
- `options` {Object} *Optional.*
  * `detailed` {Boolean} A flag to request additional fields of data with each
    account name. The default is to return account names only.
    See [Enumeration Fields](#enumfields) table below.  
    Note that the additional fields included by giving this option are not
    exhaustive; for that, see [`users.getDetails()`](#getdetails-sync) with the
    `fullDetails` option.

  * `filter` {Number} A bit-field specifying the types of user accounts to
    include (See [User Account Type Constants](#accttypeconsts) below). This
    field can be made to specify multiple types by combining them with logical OR
    (`|`) notation. The default is to request all types; a value of `0` will also
    get this result, as will a combination of all the constants.

- Return: {Array} array of {String | Object}  
  If no `options` are given, this will be an array of the names of every user
  account known to the system, including special account types if any.  
  If option `detailed` is set `true`, it will be an array of objects containing
  all of the fields in the [Enumeration Fields](#enumfields) table
  below.  
  If option `filter` is given and set non-zero, the array will include only
  the accounts with type(s) matching the specified value(s).

<a name="list-async"></a>

### users.list([options,] [hostname,] callback)
*Asynchronous.* Passes a list of all user accounts known to the local system
(or on the system named by `hostname` instead) to the `callback` function.
The contents of the list depends on whether and which options are given, as
described above for the synchronous version.
If `hostname` is given and it is unknown or cannot be accessed, an `Error` is
passed back.
- `options` {Object} *Optional.* As described for synchronous version.
- `hostname` {String} *Optional.*  
  The name of a host in the domain to query. An empty value (`undefined`, `null`,
  or empty string) may be passed to get the same effect as omitting this argument.
- `callback` {Function}  
  * **error** {Error | `null`}
  * **data** {Array} as described for return value of synchronous version,
  if no error.

<a name="getdetails-sync"></a>

### users.getDetails(username [, fullDetails])
*Synchronous.* Returns information about the named user account if it is known
on the local system; otherwise an `Error` is thrown.
- `username` {String} Logon name of user account.
- `fullDetails` {Boolean} *Optional.* Pass `true` to request all available details.
- Return: {Object} A user account record containing all of the fields listed in
  the [Enumeration Fields](#enumfields) table; if a `true` value was
  passed for the `fullDetails` parameter, the record will also contain the fields
  listed in the [Additional Fields](#fulldetails) table.

<a name="getdetails-async"></a>

### users.getDetails(username [, fullDetails] [, hostname], callback)
*Asynchronous.* Passes the account information for the named user (optionally
on the system named by `hostname`) to the `callback` function, if the username
is known; otherwise an `Error` is passed to the callback. If `hostname` is
given and it is unknown or cannot be accessed, an `Error` is passed back.
- `username` {String} Logon name of user account.
- `fullDetails` {Boolean} *Optional.* Pass `true` to request all available details.
- `hostname` {String} *Optional.*  
  The name of a host in the domain to query. An empty value (`undefined`, `null`,
  or empty string) may be passed to get the same effect as omitting this argument.
- `callback` {Function}  
  * **error** {Error | `null`}
  * **data** {Object} as described for return value of synchronous version,
    if no error.

<a name="getggroups-sync"></a>

### users.getGlobalGroups(username)
*Synchronous.* Returns the list of global groups to which the named user account
belongs, if the `username` is known on the local system; otherwise an `Error` is
thrown.
- `username` {String} Logon name of user account.
- Return: {Array} array of global group names as strings

<a name="getggroups-async"></a>

### users.getGlobalGroups(username [, hostname], callback)
*Asynchronous.* Retrieves the list of global groups to which the named user
account belongs (optionally on the system named by `hostname`) and passes it to
the `callback` function, if the username is known to the host; otherwise an
`Error` is passed to the callback. If `hostname` is given and it is unknown or
cannot be accessed, an `Error` is passed back.
- `username` {String} Logon name of user account.
- `hostname` {String} *Optional.*  
  The name of a host in the domain to query. An empty value (`undefined`, `null`,
  or empty string) may be passed to get the same effect as omitting this argument.
- `callback` {Function}  
  * **error** {Error | `null`}
  * **data** {Array} as described for return value of synchronous version,
    if no error.

<a name="getlgroups-sync"></a>

### users.getLocalGroups(username)
*Synchronous.* Returns the list of local groups to which the named user account
belongs, if the `username` is known on the local system; otherwise an `Error` is
thrown.
- `username` {String} Logon name of user account.
- Return: {Array} array of local group names as strings

<a name="getlgroups-async"></a>

### users.getLocalGroups(username [, hostname], callback)
*Asynchronous.* Retrieves the list of local groups to which the named user
account belongs (optionally on the system named by `hostname`) and passes it to
the `callback` function, if the username is known to the host; otherwise an
`Error` is passed to the callback. If `hostname` is given and it is unknown or
cannot be accessed, an `Error` is passed back.
- `username` {String} Logon name of user account.
- `hostname` {String} *Optional.*  
  The name of a host in the domain to query. An empty value (`undefined`, `null`,
  or empty string) may be passed to get the same effect as omitting this argument.
- `callback` {Function}  
  * **error** {Error | `null`}
  * **data** {Array} as described for return value of synchronous version,
    if no error.

<a name="accttypeconsts"></a>

### User Account Type Constants
- **`users.constants.NORMAL`** - Default account type, representing a typical user.
  The corresponding string value returned for the `accountType` field is `"normal"`.
- **`users.constants.TEMP_DUPLICATE`** - Account type for users whose primary
  account is in another domain. The corresponding `accountType` field value is
  `"temp duplicate"`.
- **`users.constants.INTERDOMAIN_TRUST`** - A Permit To Trust account for a domain
  that trusts other domains. The corresponding `accountType` field value is
  `"interdomain trust"`.
- **`users.constants.WORKSTATION_TRUST`** - A computer account for a computer that
  is a member of this domain. The corresponding `accountType` field value is
  `"workstation trust"`
- **`users.constants.SERVER_TRUST`** - A computer account for a backup domain
  controller that is a member of this domain. The corresponding `accountType`
  field value is `"server trust"`.

<a name="enumfields"></a>

### Enumeration Fields

<style>td { vertical-align: top; }</style>

<table>
  <thead>
    <tr><th>Field Name</th><th>Type</th><th>Description</th></tr>
  </thead>
  <tbody>
    <tr>
      <td><code>name</code></td>
      <td><code>String</code></td>
      <td>The username for account logon.</td>
    </tr>
    <tr>
      <td><code>fullName</code></td>
      <td><code>String</code></td>
      <td>Alternate identification of user; may be an empty string.</td>
    </tr>
    <tr>
      <td><code>comment</code></td>
      <td><code>String</code></td>
      <td>Descriptive comment; may be an empty string.</td>
    </tr>
    <tr>
      <td><code>accountType</code></td>
      <td><code>String</code></td>
      <td>Account usage type. See notes for <a href="#accttypeconsts">User Account Type Constants</a>.</td>
    </tr>
    <tr>
      <td><code>disabled</code></td>
      <td><code>Boolean</code></td>
      <td>The user's account is disabled.</td>
    </tr>
    <tr>
      <td><code>lockedOut</code></td>
      <td><code>Boolean</code></td>
      <td>The account is currently locked out.</td>
    </tr>
    <tr>
      <td><code>passwdRequired</code></td>
      <td><code>Boolean</code></td>
      <td>The user must use a password to log on.</td>
    </tr>
    <tr>
      <td><code>passwdCanChange</code></td>
      <td><code>Boolean</code></td>
      <td>The user can change the password.</td>
    </tr>
    <tr>
      <td><code>passwdExpired</code></td>
      <td><code>Boolean</code></td>
      <td>The user's password has expired.</td>
    </tr>
    <tr>
      <td><code>passwdNeverExpires</code></td>
      <td><code>Boolean</code></td>
      <td>Currently set for password never to expire.</td>
    </tr>
    <tr>
      <td><code>encryptedPasswdOK</code></td>
      <td><code>Boolean</code></td>
      <td>Password is stored under reversible encryption in the Active Directory.</td>
    </tr>
    <tr>
      <td><code>smartcardRequired</code></td>
      <td><code>Boolean</code></td>
      <td>The user must log on with a smart card.</td>
    </tr>
    <tr>
      <td><code>useOnlyDES</code></td>
      <td><code>Boolean</code></td>
      <td>Restricted to use only DES encryption types for keys.</td>
    </tr>
    <tr>
      <td><code>noPreauthRequired</code></td>
      <td><code>Boolean</code></td>
      <td>No Kerberos preauthentication required for logon.</td>
    </tr>
    <tr>
      <td><code>notDelegated</code></td>
      <td><code>Boolean</code></td>
      <td>Other users cannot act as delegates of this account.</td>
    </tr>
    <tr>
      <td><code>trustedForDeleg</code></td>
      <td><code>Boolean</code></td>
      <td>The account is enabled for delegation.</td>
    </tr>
    <tr>
      <td><code>trustedToAuthDeleg</code></td>
      <td><code>Boolean</code></td>
      <td>The account is trusted to authenticate a user outside of the Kerberos security package and delegate that user through constrained delegation.</td>
    </tr>
  </tbody>
</table>

> > For more information about the `Boolean` fields above, see the table of
corresponding flags under the description of the **usri20_flags** field on
[this page at MSDN.microsoft.com](https://msdn.microsoft.com/en-us/library/windows/desktop/aa371123.aspx).

<a name="fulldetails"></a>

### Additional Fields Available through `getDetails()`
<table>
  <thead>
    <tr><th>Field Name</th><th>Type</th><th>Description</th></tr>
  </thead>
  <tbody>
    <tr>
      <td><code>sid</code></td>
      <td><code>String</code></td>
      <td>The unique <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379597.aspx">security identifier (SID)</a> of the user.</td>
    </tr>
    <tr>
      <td><code>primaryGroupId</code></td>
      <td><code>Number</code></td>
      <td>The relative identifier (RID) of the Primary Global Group for the user. Information about RIDs can be found on the <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379597.aspx">MSDN page about SID Components</a>.</td>
    </tr>
    <tr>
      <td><code>homeDir</code></td>
      <td><code>String</code></td>
      <td>Path of the user's home directory; may be an empty string.</td>
    </tr>
    <tr>
      <td><code>homeDirDrive</code></td>
      <td><code>String</code></td>
      <td>The letter of the drive where the user's home directory resides; may be an empty string if <code>homeDir</code> is empty.</td>
    </tr>
    <tr>
      <td><code>profilePath</code></td>
      <td><code>String</code></td>
      <td>Path of the user's profile; can be an empty string, a local absolute path, or a UNC path.</td>
    </tr>
    <tr>
      <td><code>scriptPath</code></td>
      <td><code>String</code></td>
      <td>Path of the user's logon script file; may be an empty string.</td>
    </tr>
    <tr>
      <td><code>maxStorage</code></td>
      <td><code>Number</code></td>
      <td>If not <code>null</code>, the maximum disk space the user can use, in bytes. A <code>null</code> value means "unlimited".</td>
    </tr>
    <tr>
      <td><code>privilegeLevel</code></td>
      <td><code>String</code></td>
      <td><code>"guest"</code>, <code>"user"</code>, or <code>"administrator"</code> (see <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379306.aspx">MSDN page about Privileges</a>).</td>
    </tr>
    <tr>
      <td><code>isAccountsOperator</code></td>
      <td><code>Boolean</code></td>
      <td>Flag: the user has Accounts Operator privilege.</td>
    </tr>
    <tr>
      <td><code>isServerOperator</code></td>
      <td><code>Boolean</code></td>
      <td>Flag: the user has Print Operator privilege.</td>
    </tr>
    <tr>
      <td><code>isPrintOperator</code></td>
      <td><code>Boolean</code></td>
      <td>Flag: the user has Server Operator privilege.</td>
    </tr>
    <tr>
      <td><code>userComment</code></td>
      <td><code>String</code></td>
      <td>Another comment associated with the user; may be an empty string.</td>
    </tr>
    <tr>
      <td><code>countryCode</code></td>
      <td><code>Number</code></td>
      <td>The country/region code for the user's language of choice.</td>
    </tr>
    <tr>
      <td><code>codePage</code></td>
      <td><code>Number</code></td>
      <td>The code page for the user's language of choice.</td>
    </tr>
    <tr>
      <td><code>appParams</code></td>
      <td><code>String</code></td>
      <td>Reserved for use by some applications; may be an empty string.</td>
    </tr>
    <tr>
      <td><code>passwdAge</code></td>
      <td><code>Number</code></td>
      <td>Number of seconds passed since password was last changed. The value <code>0</code> means "never".</td>
    </tr>
    <tr>
      <td><code>accountExpires</code></td>
      <td><code>Number</code></td>
      <td>If not <code>null</code>, the time when the account expires, in number of seconds after UNIX epoch. A <code>null</code> value means "never".</td>
    </tr>
    <tr>
      <td><code>logonHours</code></td>
      <td><code>Array</code></td>
      <td>If not <code>null</code>, a 2-dimensional array identifying GMT hours of the week during which the user can log on. The outer array has an element for each day of the week, starting with Sunday; each element is an array of integers representing 0-indexed hours of the day, GMT (<a href="#logonhours">see "How To" below</a>). A <code>null</code> value means no restriction.</td>
    </tr>
    <tr>
      <td><code>workstations</code></td>
      <td><code>String</code></td>
      <td>Comma-delimited list of workstations from which the user can log on; may be an empty string, meaning: unrestricted.</td>
    </tr>
    <tr>
      <td><code>logonServer</code></td>
      <td><code>String</code></td>
      <td>Name of the server to which logon requests are sent. The value <code>"\\\\*"</code> means any logon server can handle requests. An empty string means the domain controller handles requests.</td>
    </tr>
    <tr>
      <td><code>lastLogon</code></td>
      <td><code>Number</code></td>
      <td>Time of last logon, in number of seconds after UNIX epoch; the value <code>0</code> means "never". About accuracy, see special notes for corresponding field <strong>usri4_last_logon</strong> on <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa371339.aspx">this page at MSDN.microsoft.com</a>.</td>
    </tr>
    <tr>
      <td><code>logonCount</code></td>
      <td><code>Number</code></td>
      <td>If not <code>null</code>, the number of times logon to this account was successful. A <code>null</code> value means "unknown". About accuracy, see special notes for corresponding field <strong>usri4_num_logons</strong> on <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa371339.aspx">this page at MSDN.microsoft.com</a>.</td>
    </tr>
    <tr>
      <td><code>badPasswdCount</code></td>
      <td><code>Number</code></td>
      <td>If not <code>null</code>, the number of times logon was attempted with an incorrect password. A <code>null</code> value means "unknown". About accuracy, see special notes for corresponding field <strong>usri4_bad_pw_count</strong> on <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa371339.aspx">this page at MSDN.microsoft.com</a>.</td>
    </tr>
  </tbody>
</table>

<a name="logonhours"></a>

#### **Logon Hours: How To...**
As noted above, the values in the `logonHours` structure are according to
Greenwich Mean Time (GMT). You will probably want to convert to values relative
to your local timezone.
- If your timezone offset is positive:
  1. add the offset to the value to get result A
  2. if result A is less than 24, it applies to the given day
  3. else take the modulus (result A % 24) and apply it to the following day.  
  Note that if the day index is 6, the following day would be Sunday (index 0).
- If your timezone offset is negative:
  1. add the offset to the value to get result A
  2. if result A is positive, it applies to the given day
  3. else add (result A + 24) and apply it to the previous day.  
  Note that if the day index is 0, the previous day would be Saturday (index 6).

------

**License: MIT**


