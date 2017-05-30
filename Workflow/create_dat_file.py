import os

# Get the full raw file name from the user
print('Enter raw file name of FULL dataset')
print('Example:')
print('cameroon can 2_35.7um_2k_Rec.raw')
print('C:\\datasets\cameroon can 2_35.7um_2k_Rec.raw')

filename = input()
if not filename:
    print('Non-empty filename expected')
    exit()

if filename[:-3] == 'raw':
    print('Raw file expected')
    exit()

print()

######
# Get the full volume dimensions from the user
print('Enter FULL dataset volume dimensions, separated by spaces')
print('Example:')
print('2224 2224 4580')

number_strings = input().split(' ')
if len(number_strings) != 3:
    print('Three numbers expected')
    exit()

x_dim = int(number_strings[0])
y_dim = int(number_strings[1])
z_dim = int(number_strings[2])

# The basis vectors should be a bit bigger for ease of use, so we normalize
# them to being 10 at least

# Find the minimum
w = min(s for s in [x_dim, y_dim, z_dim])

# Normalize all values
basis_x_dim = float(x_dim) / float(w) * 10.0
basis_y_dim = float(y_dim) / float(w) * 10.0
basis_z_dim = float(z_dim) / float(w) * 10.0


dat_file_path = filename[:-3] + 'dat'
with open(dat_file_path, 'w') as dat_file:
    dat_file.write('Rawfile: {}\n'.format(os.path.basename(os.path.normpath(filename))))
    dat_file.write('Resolution: {} {} {}\n'.format(x_dim, y_dim, z_dim))
    dat_file.write('Format: UINT8\n')
    dat_file.write('BasisVector1: {} 0.0 0.0\n'.format(basis_x_dim))
    dat_file.write('BasisVector2: 0.0 {} 0.0\n'.format(basis_y_dim))
    dat_file.write('BasisVector3: 0.0 0.0 {}\n'.format(basis_z_dim))





# Get the scaled raw file name from the user
print('Enter raw file name of SCALED dataset')
print('Example:')
print('cameroon can 2_35.7um_2k_Rec-scaled.raw')
print('C:\\datasets\cameroon can 2_35.7um_2k_Rec-scaled.raw')

filename = input()
if not filename:
    print('Non-empty filename expected')
    exit()

if filename[:-3] == 'raw':
    print('Raw file expected')
    exit()

print()


######
# Get the scaled volume dimensions from the user
print('Enter SCALED dataset volume dimensions, separated by spaces')
print('Example:')
print('256 256 527')

number_strings = input().split(' ')
if len(number_strings) != 3:
    print('Three numbers expected')
    exit()

x_dim = int(number_strings[0])
y_dim = int(number_strings[1])
z_dim = int(number_strings[2])

# The basis vectors should be a bit bigger for ease of use, so we normalize
# them to being 10 at least

# Find the minimum
w = min(s for s in [x_dim, y_dim, z_dim])

# Normalize all values
basis_x_dim = float(x_dim) / float(w) * 10.0
basis_y_dim = float(y_dim) / float(w) * 10.0
basis_z_dim = float(z_dim) / float(w) * 10.0


dat_file_path = filename[:-3] + 'dat'
with open(dat_file_path, 'w') as dat_file:
    dat_file.write('Rawfile: {}\n'.format(os.path.basename(os.path.normpath(filename))))
    dat_file.write('Resolution: {} {} {}\n'.format(x_dim, y_dim, z_dim))
    dat_file.write('Format: UINT8\n')
    dat_file.write('BasisVector1: {} 0.0 0.0\n'.format(basis_x_dim))
    dat_file.write('BasisVector2: 0.0 {} 0.0\n'.format(basis_y_dim))
    dat_file.write('BasisVector3: 0.0 0.0 {}\n'.format(basis_z_dim))
